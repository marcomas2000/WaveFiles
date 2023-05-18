#ifndef WAVEINSPECTOR_H
#define WAVEINSPECTOR_H

#include <QMainWindow>
#include <QString>
#include "WaveFileHandler.h"

namespace Ui {
class WaveInspector;
}

class WaveInspector : public QMainWindow
{
    Q_OBJECT

public:
    explicit WaveInspector(QWidget *parent = 0);
    ~WaveInspector();
signals:
    void fileSelected(QString fileName);

private slots:
    void on_actionOpen_triggered();

    void on_actionClose_triggered();

    void on_actionFind_FMT_triggered();

    void on_actionFind_data_triggered();

    void on_actionExit_triggered();

private:
    Ui::WaveInspector *ui;
    CWaveFileHandler * m_fh;
};

#endif // WAVEINSPECTOR_H
