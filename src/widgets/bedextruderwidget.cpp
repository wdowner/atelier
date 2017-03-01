#include "bedextruderwidget.h"
#include "ui_bedextruderwidget.h"
#include <QRadioButton>
#include <KLocalizedString>
BedExtruderWidget::BedExtruderWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::BedExtruderWidget)
{
    ui->setupUi(this);

    //Add Default Extruder
    setExtruderCount(1);
    connect(ui->heatBedPB, &QPushButton::clicked, [ = ](bool clicked) {
        uint temp = ui->bedTempSB->value() * clicked;
        emit bedTemperatureChanged(temp);
        ui->bedTargetTempLB->setText(QString::number(temp));
    });

    connect(ui->heatExtPB, &QPushButton::clicked, this, &BedExtruderWidget::heatExtruderClicked);
}

BedExtruderWidget::~BedExtruderWidget()
{
    delete ui;
}

void BedExtruderWidget::setExtruderCount(int value)
{
    if (value == extruderCount) {
        return;
    } else if (extruderCount < value) {
        //loop for the new buttons
        for (int i = extruderCount; i < value; i++) {
            auto *rb = new QRadioButton(QString::number(i+1));
            ui->extRadioButtonLayout->addWidget(rb);
            extruderMap.insert(i, rb);
        }
    } else {
        //remove buttons - need to test it!
        for (int i = extruderCount; i >= value; i--) {
            auto *rb = extruderMap.value(i);
            ui->extRadioButtonLayout->removeWidget(rb);
            extruderMap.remove(i);
            delete(rb);
        }
    }
    extruderCount = value;
}

void BedExtruderWidget::updateBedTemp(const float temp)
{
    ui->bedCurrTempLB->setText(QString::number(temp));
}

void BedExtruderWidget::updateExtTemp(const float temp)
{
    ui->extCurrTempLB->setText(QString::number(temp));
}

void BedExtruderWidget::heatExtruderClicked(bool clicked)
{
    int currExt = 1;
    if (extruderMap.size() > 1) {
        for (int i = 0; i < extruderMap.size(); i++) {
            if (extruderMap.value(i)->isChecked()) {
                currExt = i;
                break;
            }
        }
    }
    uint tmp = ui->extTempSB->value() * clicked;
    emit extTemperatureChanged(currExt, tmp);
    ui->extTargetTempLB->setText(QString::number(tmp));
}