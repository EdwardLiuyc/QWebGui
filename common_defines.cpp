#include "common_defines.h"

void paintACoordSystem(QPainter *painter, QPoint &org)
{
    const int length = 30;
    if( org.x() < -length || org.y() < -length )
        return;

    QPoint left, right, top, btm;
    left.setX( org.x() - length );
    left.setY( org.y() );
    right.setX( org.x()+ length);
    right.setY( org.y());
    top.setX( org.x() );
    btm.setX( org.x() );
    top.setY( org.y() - length );
    btm.setY( org.y() + length );
    QPen pen( Qt::green, 2, Qt::DashDotLine, Qt::RoundCap, Qt::RoundJoin );
    painter->setPen(pen);
    painter->drawLine(left, right);
    painter->drawLine(top, btm);
}

void PaintARobot(QPainter *painter, QPoint pos, double yaw, double factor)
{
    if( !painter || factor < 1.0e-6 || pos.x() < 0 || pos.y() < 0)
        return;

    const int header_lth = 20;
    const int tail_lth = 12;

    factor = 1.0; // set robot static
    QPoint header, left, right;
    header.setX( pos.x() + header_lth*cos(yaw) * factor);
    header.setY( pos.y() - header_lth*sin(yaw) * factor);   // in screen, the coord system is different, so "-"
    double left_angle = yaw+2.356;
    double right_angle = yaw-2.356;
    left.setX( pos.x() + tail_lth*cos(left_angle) * factor);
    left.setY( pos.y() - tail_lth*sin(left_angle) * factor);
    right.setX( pos.x() + tail_lth*cos(right_angle) * factor);
    right.setY( pos.y() - tail_lth*sin(right_angle) * factor);

    painter->drawLine(left, pos);
    painter->drawLine(left, header);
    painter->drawLine(right, pos);
    painter->drawLine(right, header);
}

QPoint CalculateScreenPos(QPointF robot_pos, double resolution, QPoint origin, double factor )
{
    return QPoint( origin.x() + robot_pos.x() / resolution * factor
                   , origin.y() - robot_pos.y() / resolution * factor );
}
