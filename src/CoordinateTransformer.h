#pragma once

#include <QPointF>
#include <cmath>

class CoordinateTransformer {
public:
    CoordinateTransformer() = default;

    // Convert geographic Lat/Lon coordinates into standard Web Mercator projected coordinates (meters)
    static QPointF project(double lat, double lon) {
        const double rMajor = 6378137.0; // Equatorial radius in meters
        double x = rMajor * (lon * M_PI / 180.0);
        double latRad = lat * M_PI / 180.0;
        double y = rMajor * std::log(std::tan(M_PI / 4.0 + latRad / 2.0));
        return QPointF(x, y);
    }
};
