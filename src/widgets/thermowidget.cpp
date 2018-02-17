#include "thermowidget.h"
#include <QKeyEvent>
#include <QPaintEvent>
#include <QFocusEvent>
#include <QPainter>
#include <QBrush>
#include <QPen>
#include <QWheelEvent>

ThermoWidget::ThermoWidget(QWidget *parent, QString name) : QwtDial(parent),
    m_currentTemperatureNeedle(new QwtDialSimpleNeedle(QwtDialSimpleNeedle::Arrow)),
    m_targetTemperatureNeedle(new QwtDialSimpleNeedle(QwtDialSimpleNeedle::Arrow, Qt::red, Qt::darkRed)),
    m_name(name),
    m_currentTemperature(0),
    m_targetTemperature(0)
{
    setScaleArc(40, 320);
    setNeedle(m_currentTemperatureNeedle);
    setReadOnly(false);
    setFocusPolicy(Qt::StrongFocus);
}

void ThermoWidget::keyPressEvent(QKeyEvent* event)
{
    if (event->key() >= Qt::Key_0 && event->key() <= Qt::Key_9) {
        auto tmp = m_currentTemperatureTextFromEditor + event->key();
        if (tmp.toInt() <= upperBound() && tmp.toInt() >= lowerBound()) {
            m_currentTemperatureTextFromEditor = tmp;
        }
    } else if (event->key() == Qt::Key_Backspace && m_currentTemperatureTextFromEditor.count()) {
        m_currentTemperatureTextFromEditor.remove(m_currentTemperatureTextFromEditor.count() - 1, 1);
    } else if (event->key() == Qt::Key_Enter) {
        m_targetTemperature = m_currentTemperatureTextFromEditor.toInt();
    } else if (event->key() == Qt::Key_Escape) {
        m_currentTemperatureTextFromEditor = '0';
    } else if (event->key() == Qt::Key_Up || event->key() == Qt::Key_Right) {
        if (m_targetTemperature != upperBound()) {
            m_currentTemperatureTextFromEditor = QString::number(m_targetTemperature + 1);
        }
    } else if (event->key() == Qt::Key_Down || event->key() == Qt::Key_Left) {
        if (m_targetTemperature != lowerBound()) {
            m_currentTemperatureTextFromEditor = QString::number(m_targetTemperature - 1);
        }
    } else if (event->key() == Qt::Key_PageUp) {
        if (m_targetTemperature + 10 > upperBound()) {
            m_currentTemperatureTextFromEditor = QString::number(upperBound());
        } else {
            m_currentTemperatureTextFromEditor = QString::number(m_targetTemperature + 10);
        }
    } else if (event->key() == Qt::Key_PageDown) {
        if (m_targetTemperature - 10 < lowerBound()) {
            m_currentTemperatureTextFromEditor = QString::number(lowerBound());
        } else {
            m_currentTemperatureTextFromEditor = QString::number(m_targetTemperature - 10);
        }
    }

    else {
        QwtDial::keyPressEvent(event);
        return;
    }

    if (m_targetTemperature != m_currentTemperatureTextFromEditor.toInt()) {
        m_targetTemperature = m_currentTemperatureTextFromEditor.toInt();
        emit targetTemperatureChanged(m_targetTemperature);
        update();
        event->accept();
    }
}

void ThermoWidget::wheelEvent(QWheelEvent* event)
{
    if (event->angleDelta().y() > 0) {
        if (m_targetTemperature + 10 > upperBound()) {
            m_currentTemperatureTextFromEditor = QString::number(upperBound());
        } else {
            m_currentTemperatureTextFromEditor = QString::number(m_targetTemperature + 10);
        }
    } else if (event->angleDelta().y() < 0) {
        if (m_targetTemperature - 10 < lowerBound()) {
            m_currentTemperatureTextFromEditor = QString::number(lowerBound());
        } else {
            m_currentTemperatureTextFromEditor = QString::number(m_targetTemperature - 10);
        }
    }

    if (m_targetTemperature != m_currentTemperatureTextFromEditor.toInt()) {
        m_targetTemperature = m_currentTemperatureTextFromEditor.toInt();
        emit targetTemperatureChanged(m_targetTemperature);
        update();
    }
    event->accept();
}

void ThermoWidget::focusOutEvent(QFocusEvent* event)
{
    if (m_targetTemperature != m_currentTemperatureTextFromEditor.toInt()) {
        m_targetTemperature = m_currentTemperatureTextFromEditor.toInt();
        emit targetTemperatureChanged(m_targetTemperature);
        update();
        event->accept();
    }
}

void ThermoWidget::paintEvent(QPaintEvent* event)
{
    QwtDial::paintEvent(event);
    const QString currentText = QString::number(m_currentTemperature);

    QFontMetrics fm(font());
    const double targetWidth = fm.width(m_currentTemperatureTextFromEditor);
    const double currentWidth = fm.width(currentText);
    const double nameWidth = fm.width(m_name);

    const double height = fm.height();
    const double halfWidth = geometry().width() / 2;
    const double xposTarget = halfWidth - (targetWidth / 2);
    const double xposCurrent = halfWidth - (currentWidth / 2);
    const double xposName = halfWidth - (nameWidth / 2);
    const double ypos = geometry().height() - height * 2  - 2;

    QPainter p(this);

    QColor color = palette().color(QPalette::Text);

    p.setPen(Qt::red);
    p.drawText(xposTarget, ypos - 2 * height, m_currentTemperatureTextFromEditor);

    p.setPen(color);
    p.drawText(xposCurrent, ypos - height, QString::number(m_currentTemperature));

    p.setPen(color);
    p.drawText(xposName, ypos, m_name);
}

void ThermoWidget::drawNeedle( QPainter *painter, const QPointF &center, double radius, double dir, QPalette::ColorGroup colorGroup ) const
{
    Q_UNUSED( dir );

    const double relativePercent = upperBound() - lowerBound();
    const double currentTemperaturePercent = (m_currentTemperature - lowerBound()) / relativePercent;
    const double targetTemperaturePercent = (m_targetTemperature - lowerBound()) / relativePercent;
    const double currentTemperatureAngle = (maxScaleArc() - minScaleArc()) * currentTemperaturePercent + minScaleArc();
    const double targetTemperatureAngle = (maxScaleArc() - minScaleArc()) * targetTemperaturePercent + minScaleArc();

    // Qt coordinates and Qwt coordinates differ.
    // the "begin" of our coordinates in Qt: -130
    // the "span" of our coordinates in Qt: -180
    // Negative values means clockwise in Qt dialect.
    const double qtBeginAngle = -130;
    const double coolZone =  - (targetTemperatureAngle - minScaleArc());
    int yPos = geometry().height() / 2 - radius;
    int xPos = geometry().width() / 2 - radius;

    QRadialGradient grad(center,radius);
    grad.setColorAt(0.75,QColor(0,0,0,0));
    grad.setColorAt(0.85,QColor(255,0,0,196));
    grad.setColorAt(0.95,QColor(255,110,60,196));

    painter->setBrush(grad);
    painter->drawPie(xPos,yPos, radius * 2, radius * 2, qtBeginAngle * 16, coolZone* 16);

    m_targetTemperatureNeedle->draw(painter, center, radius * 1.3, 360 - targetTemperatureAngle - origin(), colorGroup);
    m_currentTemperatureNeedle->draw(painter, center, radius, 360 - currentTemperatureAngle - origin(), colorGroup);
}

void ThermoWidget::setCurrentTemperature(double temperature)
{
    if (m_currentTemperature != temperature) {
        m_currentTemperature = temperature;
        update();
    }
}

void ThermoWidget::setTargetTemperature(double temperature)
{
    if (m_targetTemperature != temperature) {
        m_currentTemperatureTextFromEditor = QString::number(temperature);
        m_targetTemperature = temperature;
        emit targetTemperatureChanged(m_targetTemperature);
        update();
    }
}
