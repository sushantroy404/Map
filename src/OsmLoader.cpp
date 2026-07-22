#include "OsmLoader.h"
#include "CoordinateTransformer.h"
#include <QFile>
#include <QXmlStreamReader>
#include <QDebug>
#include <Set>

// Haversine distance helper in meters
static double calculateDistance(double lat1, double lon1, double lat2, double lon2) {
    const double R = 6371000.0; // earth radius
    double dLat = (lat2 - lat1) * M_PI / 180.0;
    double dLon = (lon2 - lon1) * M_PI / 180.0;
    double a = std::sin(dLat / 2) * std::sin(dLat / 2) +
               std::cos(lat1 * M_PI / 180.0) * std::cos(lat2 * M_PI / 180.0) *
               std::sin(dLon / 2) * std::sin(dLon / 2);
    double c = 2 * std::atan2(std::sqrt(a), std::sqrt(1 - a));
    return R * c;
}

bool OsmLoader::load(const QString &filePath, Graph &graph, QString &errorMessage) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        errorMessage = "Could not open file: " + file.errorString();
        return false;
    }

    graph.clear();

    QXmlStreamReader xml(&file);
    struct RawNode {
        QString id;
        double lat;
        double lon;
    };

    QMap<QString, RawNode> rawNodes;
    std::set<QString> nodesUsedInWays;

    struct RawWay {
        QString id;
        QVector<QString> nodeRefs;
        QString name;
        QString type;
    };
    QVector<RawWay> rawWays;

    // Parse loop
    while (!xml.atEnd() && !xml.hasError()) {
        QXmlStreamReader::TokenType token = xml.readNext();
        if (token == QXmlStreamReader::StartElement) {
            if (xml.name().toString() == "node") {
                QString id = xml.attributes().value("id").toString();
                double lat = xml.attributes().value("lat").toDouble();
                double lon = xml.attributes().value("lon").toDouble();
                rawNodes[id] = {id, lat, lon};
            } else if (xml.name().toString() == "way") {
                QString wayId = xml.attributes().value("id").toString();
                QVector<QString> ndRefs;
                QString streetName;
                QString highwayType;
                bool isHighway = false;

                // Read inner tags
                while (xml.readNextStartElement()) {
                    if (xml.name().toString() == "nd") {
                        ndRefs.append(xml.attributes().value("ref").toString());
                        xml.skipCurrentElement();
                    } else if (xml.name().toString() == "tag") {
                        QString k = xml.attributes().value("k").toString();
                        QString v = xml.attributes().value("v").toString();
                        if (k == "highway") {
                            isHighway = true;
                            highwayType = v;
                        } else if (k == "name") {
                            streetName = v;
                        }
                        xml.skipCurrentElement();
                    } else {
                        xml.skipCurrentElement();
                    }
                }

                if (isHighway && ndRefs.size() >= 2) {
                    rawWays.append({wayId, ndRefs, streetName, highwayType});
                    for (const QString &ref : ndRefs) {
                        nodesUsedInWays.insert(ref);
                    }
                }
            }
        }
    }

    if (xml.hasError()) {
        errorMessage = "XML parser error: " + xml.errorString();
        return false;
    }

    if (rawWays.isEmpty()) {
        errorMessage = "No valid road networks (<way> tagged with 'highway') discovered inside OSM file.";
        return false;
    }

    // Identify bounds to normalize coordinates dynamically
    double minLat = std::numeric_limits<double>::infinity();
    double maxLat = -std::numeric_limits<double>::infinity();
    double minLon = std::numeric_limits<double>::infinity();
    double maxLon = -std::numeric_limits<double>::infinity();

    for (const QString &nodeId : nodesUsedInWays) {
        if (rawNodes.contains(nodeId)) {
            const RawNode &node = rawNodes[nodeId];
            if (node.lat < minLat) minLat = node.lat;
            if (node.lat > maxLat) maxLat = node.lat;
            if (node.lon < minLon) minLon = node.lon;
            if (node.lon > maxLon) maxLon = node.lon;
        }
    }

    // Mercator dynamic bounds
    QPointF minProj = CoordinateTransformer::project(minLat, minLon);
    QPointF maxProj = CoordinateTransformer::project(maxLat, maxLon);
    double widthProj = maxProj.x() - minProj.x();
    double heightProj = maxProj.y() - minProj.y();
    double maxDimension = std::max(widthProj, heightProj);
    double targetScale = 1000.0; // scale space

    // Build Graph Nodes
    for (const QString &nodeId : nodesUsedInWays) {
        if (rawNodes.contains(nodeId)) {
            const RawNode &raw = rawNodes[nodeId];
            QPointF proj = CoordinateTransformer::project(raw.lat, raw.lon);
            
            // Normalize screen scale coordinates
            double sx = ((proj.x() - minProj.x()) / maxDimension) * targetScale;
            double sy = targetScale - ((proj.y() - minProj.y()) / maxDimension) * targetScale;
            
            Node node{raw.id, raw.lat, raw.lon, QPointF(sx, sy)};
            graph.addNode(node);
        }
    }

    // Build Graph Edges (segments of roads)
    int edgeCounter = 0;
    for (const RawWay &way : rawWays) {
        for (int i = 0; i < way.nodeRefs.size() - 1; ++i) {
            QString src = way.nodeRefs[i];
            QString dest = way.nodeRefs[i+1];

            if (graph.hasNode(src) && graph.hasNode(dest)) {
                const Node &n1 = graph.getNode(src);
                const Node &n2 = graph.getNode(dest);

                double dist = calculateDistance(n1.lat, n1.lon, n2.lat, n2.lon);
                double speedFactor = 1.0;
                
                // Set speed weights
                if (way.type == "primary") speedFactor = 1.5;
                else if (way.type == "footway") speedFactor = 0.5;
                
                double weight = dist / speedFactor;
                QString edgeId = QString("%1_seg_%2").arg(way.id).arg(i);
                
                Edge edge{
                    edgeId,
                    src,
                    dest,
                    weight,
                    dist,
                    way.name.isEmpty() ? QString("Street %1").arg(way.id.right(4)) : way.name,
                    way.type
                };
                graph.addEdge(edge);
            }
        }
    }

    return true;
}
