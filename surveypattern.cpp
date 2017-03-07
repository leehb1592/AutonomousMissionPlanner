#include "surveypattern.h"
#include "waypoint.h"
#include <QPainter>
#include <QtMath>

SurveyPattern::SurveyPattern(QObject *parent, QGraphicsItem *parentItem):GeoGraphicsItem(parent, parentItem),
    m_startLocation(nullptr),m_endLocation(nullptr),m_spacingLocation(nullptr)
{

}

void SurveyPattern::setStartLocation(const QGeoCoordinate &location)
{
    if(m_startLocation == nullptr)
    {
        m_startLocation = new Waypoint(parent(),this);
        m_startLocation->setFlag(QGraphicsItem::ItemIsMovable);
        m_startLocation->setFlag(QGraphicsItem::ItemIsSelectable);
        m_startLocation->setFlag(QGraphicsItem::ItemSendsGeometryChanges);
        m_startLocation->setFlag(QGraphicsItem::ItemSendsScenePositionChanges);
    }
    m_startLocation->setLocation(location);
    m_startLocation->setPos(m_startLocation->geoToPixel(location));
    update();
}

void SurveyPattern::setEndLocation(const QGeoCoordinate &location)
{
    if(m_endLocation == nullptr)
    {
        m_endLocation = new Waypoint(parent(),this);
        m_endLocation->setFlag(QGraphicsItem::ItemIsMovable);
        m_endLocation->setFlag(QGraphicsItem::ItemIsSelectable);
        m_endLocation->setFlag(QGraphicsItem::ItemSendsGeometryChanges);
        m_endLocation->setFlag(QGraphicsItem::ItemSendsScenePositionChanges);
    }
    m_endLocation->setLocation(location);
    m_endLocation->setPos(m_endLocation->geoToPixel(location));
    update();
}

void SurveyPattern::setSpacingLocation(const QGeoCoordinate &location)
{
    if(m_spacingLocation == nullptr)
    {
        m_spacingLocation = new Waypoint(parent(),this);
        m_spacingLocation->setFlag(QGraphicsItem::ItemIsMovable);
        m_spacingLocation->setFlag(QGraphicsItem::ItemIsSelectable);
        m_spacingLocation->setFlag(QGraphicsItem::ItemSendsGeometryChanges);
        m_spacingLocation->setFlag(QGraphicsItem::ItemSendsScenePositionChanges);
    }
    m_spacingLocation->setLocation(location);
    m_spacingLocation->setPos(m_spacingLocation->geoToPixel(location));
    update();
}


bool SurveyPattern::hasSpacingLocation() const
{
    return (m_spacingLocation != nullptr);
}

QRectF SurveyPattern::boundingRect() const
{
    //return childrenBoundingRect();
    return shape().boundingRect();
}

void SurveyPattern::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    auto children = getPath();
    if (children.length() > 1)
    {
        painter->save();

        painter->setPen(Qt::red);


        auto first = children.begin();
        auto second = first;
        second++;
        while(second != children.end())
        {
            painter->drawLine(m_startLocation->geoToPixel(*first),m_startLocation->geoToPixel(*second));
            first++;
            second++;
        }

        painter->restore();

    }

}

QPainterPath SurveyPattern::shape() const
{
    auto children = getPath();
    auto i = children.begin();
    QPainterPath ret(m_startLocation->geoToPixel(*i));
    i++;
    while(i != children.end())
    {
        ret.lineTo(m_startLocation->geoToPixel(*i));
        i++;
    }
    QPainterPathStroker pps;
    pps.setWidth(5);
    return pps.createStroke(ret);

}

QList<QGeoCoordinate> SurveyPattern::getPath() const
{
    QList<QGeoCoordinate> ret;
    if(m_startLocation)
    {
        ret.append(m_startLocation->location());
        QGeoCoordinate lastLocation = ret.back();
        if(m_endLocation)
        {
            qreal ab_distance = m_startLocation->location().distanceTo(m_endLocation->location());
            qreal ab_angle = m_startLocation->location().azimuthTo(m_endLocation->location());

            qreal ac_distance = 1.0;
            qreal ac_angle = 90.0;
            if(m_spacingLocation)
            {
                ac_distance = m_startLocation->location().distanceTo(m_spacingLocation->location());
                ac_angle = m_startLocation->location().azimuthTo(m_spacingLocation->location());
            }
            qreal leg_heading = ac_angle-90.0;
            qreal leg_length = ab_distance*qCos(qDegreesToRadians(ab_angle-leg_heading));
            qreal surveyWidth = ab_distance*qSin(qDegreesToRadians(ab_angle-leg_heading));

            int line_count = qCeil(surveyWidth/ac_distance);
            for (int i = 0; i < line_count; i++)
            {
                int dir = i%2;
                ret.append(lastLocation.atDistanceAndAzimuth(leg_length,leg_heading+dir*180));
                ret.append(ret.back().atDistanceAndAzimuth(ac_distance,ac_angle));
                lastLocation = ret.back();
            }
            if (ret.length() < 2)
                ret.append(m_endLocation->location());
        }
    }
    return ret;
}