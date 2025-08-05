#ifndef LED_H
#define LED_H

#include <QWidget>
#include <QDebug>
#include <QFile>

class led : public QWidget
{
    Q_OBJECT
public:
    explicit led(QWidget *parent = nullptr);


    void setLedState();//set LED
    bool getLedState();//get LED

signals:


private:
    QFile file;
};

#endif // LED_H
