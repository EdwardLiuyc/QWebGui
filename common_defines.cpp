#include "common_defines.h"
#include <QtMath>

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

void PaintARobot(QPainter *painter, QPointF pos, double yaw, double factor)
{
    if( !painter || factor < 1.0e-6 || pos.x() < 0 || pos.y() < 0)
        return;

    const int header_lth = 20;
    const int tail_lth = 12;

    factor = 1.0; // set robot static
    QPointF header, left, right;
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

QPointF CalculateScreenPos(QPointF robot_pos, double resolution, QPoint origin, double factor )
{
    return QPointF( origin.x() + robot_pos.x() / resolution * factor
                   , origin.y() - robot_pos.y() / resolution * factor );
}

void PaintADot(QPainter* painter, QPointF &pos )
{
    if( !painter || pos.x() < 0 || pos.y() < 0 )
        return;

    const int radius = 3;
    painter->setBrush( QBrush(QColor(Qt::red)));
    painter->drawEllipse( pos, radius, radius );
}

void PaintATargetPoint( QPainter* painter, QPointF& pos )
{
    if( !painter || pos.x() < 0 || pos.y() < 0 )
        return;

    QPointF tmp_point = pos + QPointF(4,-10);
    QColor color( QColor(Qt::red) );
    painter->setPen( QPen(color, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin) );
    painter->drawLine( pos, tmp_point );

    QPainterPath path;
    path.moveTo( tmp_point );
    path.lineTo( tmp_point + QPointF( 8, 0 ) );
    path.lineTo( tmp_point + QPointF( 8, 0 ) + QPointF( -1, 5 ));
    path.lineTo( pos + QPointF( 2, -5 ) );
    painter->fillPath( path, color );
}

bool tooClose( RobotState& a, RobotState& b, double min_distance )
{
    return sqrt( pow(a.x-b.x, 2.) + pow(a.y-b.y, 2. ) ) < min_distance;
}

void PaintRunningHistory(QPainter* painter, std::list<RobotState> &history, double robot_width, double resolution, double factor, QPoint origin)
{
    // at least 2 history state
    if( !painter || history.size() <= 1 || robot_width <= 1.e-6 )
        return;

    std::list<RobotState>::iterator it = history.begin();
    RobotState last_state = *it;
    RobotState current_state;
    double width_2 = robot_width * 0.5;
    QColor color( QColor(Qt::gray) );
//    painter->setPen( color );
    for( std::advance( it, 1 );
         it != history.end();
         ++it)
    {
        QVector<QPointF> points;
        current_state = *it;
        if( tooClose( current_state, last_state, resolution) )
            continue;

        QPointF last_left( last_state.x + width_2 * cos(last_state.yaw+M_PI_2)
                         , last_state.y + width_2 * sin(last_state.yaw+M_PI_2) );
        QPointF last_right( last_state.x + width_2 * cos(last_state.yaw-M_PI_2)
                          , last_state.y + width_2 * sin(last_state.yaw-M_PI_2) );
        QPointF current_left( current_state.x + width_2 * cos(current_state.yaw+M_PI_2)
                            , current_state.y + width_2 * sin(current_state.yaw+M_PI_2) );
        QPointF current_right( current_state.x + width_2 * cos(current_state.yaw-M_PI_2)
                             , current_state.y + width_2 * sin(current_state.yaw-M_PI_2) );

        points.push_back( CalculateScreenPos(last_left, resolution, origin, factor)  );
        points.push_back( CalculateScreenPos(current_left, resolution, origin, factor) );
        points.push_back( CalculateScreenPos(current_right, resolution, origin, factor) );
        points.push_back( CalculateScreenPos(last_right, resolution, origin, factor) );

        QPainterPath path;
        path.moveTo( points.at(0));
        for( int i = 1; i < 4; ++i )
            path.lineTo( points.at(i) );

        painter->fillPath( path, color );
        last_state = current_state;
    }
}

void PaintASelectedArea(QPainter* painter, QList<QPointF> &points)
{
    if( !painter || points.size() <= 1 )
        return;

    QPainterPath path;
    path.moveTo( points.at(0) );
    for( int i = 1; i < points.size(); ++i )
        path.lineTo( points.at(i) );

    path.closeSubpath();
    painter->drawPath( path );
}

void PaintASelectedMapArea( QPainter* painter, QList<QPointF>& points_in_map, double resolution, QPoint origin, double factor )
{
    if( !painter || points_in_map.size() <= 1 )
        return;

    QList<QPointF> points_in_screen;
    for( int i = 0; i < points_in_map.size(); ++i )
        points_in_screen.append( CalculateScreenPos(points_in_map.at(i), resolution, origin, factor ) );

    PaintASelectedArea(painter, points_in_screen);
}

bool IsInsidePoly( const QPointF &iPoint, const QList<QPointF> &polygon )
{
    if( polygon.size() < 2 )
        return false;

    qreal x = iPoint.x();
    qreal y = iPoint.y();
    int32_t left = 0;
    int32_t right = 0;
    int32_t j = polygon.size()-1;

    for(int i = 0; i < polygon.size(); ++i )
    {
        if( (polygon.at(i).y() < y && polygon.at(j).y() >= y )
            || (polygon.at(j).y() < y && polygon.at(i).y() >= y) )
        {
            if((y-polygon[i].y())*(polygon[i].x()-polygon[j].x())/(polygon[i].y()-polygon[j].y())+polygon[i].x()<x)
                left++;
            else
                right++;
        }
        j = i;
    }

    return left&right;

}

QPointF CalculateRobotPos( QPointF screen_pos, double resolution, QPoint origin, double factor )
{
    QPointF tmp_vector = screen_pos - origin;
    tmp_vector *= resolution;
    tmp_vector /= factor;

    return QPointF( tmp_vector.x(), -tmp_vector.y() );
}

double CalculateDistance( QPointF& point1, QPointF& point2 )
{
    double delta_x = point1.x() - point2.x();
    double delta_y = point1.y() - point2.y();

    return qSqrt( delta_x * delta_x + delta_y * delta_y );
}
