#include "waveinspector.h"
#include "ui_waveinspector.h"
#include <QFileDialog>
#include <QLineEdit>
#include <QMessageBox>

WaveInspector::WaveInspector(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::WaveInspector), m_fh(0)
{
    ui->setupUi(this);
}

WaveInspector::~WaveInspector()
{
    delete ui;
}

void WaveInspector::on_actionOpen_triggered()
{
    ui->textEdit->clear();
    ui->label_3->clear();
    ui->lineEdit->clear();
    QString fileName = QFileDialog::getOpenFileName(this);
    if (!fileName.isEmpty())
    {
        ui->lineEdit->setText(fileName);
        m_fh = new CWaveFileHandler;
        bool ret = m_fh->OpenForRead(fileName.toUtf8());
        if (ret == true)
        {
            if (m_fh->isValidRiffHeader() == true)
            {
                RIFF_HEADER * riff = m_fh->getRiffHeader();
                char buffer[100];
                if (m_fh->getWavFileSize() == riff->dwRiffSize + 8)
                {
                    sprintf(buffer, "%4.4s %04d %4.4s", riff->szRiffID, riff->dwRiffSize+8, riff->szRiffFormat);
                    ui->textEdit->setTextBackgroundColor("#ff0000");
                    ui->textEdit->setText(buffer);
                    ui->label_3->setText("Open");
                    ui->actionFind_FMT->setEnabled(true);
                    ui->actionFind_data->setEnabled(true);
                    ret = true;
                }
                else
                {
                    ui->label_3->setText("Incorrect file size");
                    sprintf(buffer, "WAV file seems to be corrupted");
                    ret = false;
                }
            }
            else
            {
                ui->label_3->setText("RIFF incorrect");
                ret = false;
            }
        }
        else
        {
            ui->label_3->setText("Open failure");
        }
    }
}

void WaveInspector::on_actionClose_triggered()
{
    ui->actionFind_FMT->setEnabled(false);
    ui->actionFind_data->setEnabled(false);
    ui->lineEdit->clear();
    ui->textEdit->clear();
    ui->label_3->clear();
}

void WaveInspector::on_actionFind_FMT_triggered()
{
    qint64 fmtFound = m_fh->FindValidFmtChunk();
    if (fmtFound >= 0)
    {
        char buffer[200];
        sprintf(buffer, "FMT chunk found");
        ui->label_3->setText(buffer);
        FMT_BLOCK * fmt = m_fh->getFmtHeader();
        sprintf(buffer, "%4.4s Uncompressed - No. Channels: %d - Hz: %d - Avg. bps: %d - Align: %d - Bit per Sample: %d",
                         fmt->szFmtID,
                         fmt->wavFormat.wChannels,
                         fmt->wavFormat.dwSamplesPerSec,
                         fmt->wavFormat.dwAvgBytesPerSec,
                         fmt->wavFormat.wBlockAlign,
                         fmt->wavFormat.wBitsPerSample
                         );
        ui->textEdit->setTextBackgroundColor("#00ffff");
        ui->textEdit->append(buffer);

    }
}

void WaveInspector::on_actionFind_data_triggered()
{
    qint64 dataFound = m_fh->FindValidDataChunk();
    if (dataFound >= 0)
    {
        char buffer[200];
        sprintf(buffer, "DATA chunk found");
        ui->label_3->setText(buffer);
        DATA_BLOCK * data = m_fh->getDataBlock();
        sprintf(buffer, "%4.4s Size: %d - No. of secs: %d",
                         data->szDataID,
                         data->dwDataSize,
                         m_fh->lengthOfAudioSample()
                         );
        ui->textEdit->setTextBackgroundColor("#00ff00");
        ui->textEdit->append(buffer);
        m_fh->analyzedataChunk();
        AUDIO_CHANNEL * sample_analysis = m_fh->getAudioChannels();
        if (sample_analysis != 0)
        {
            for (int i=0; i < m_fh->lengthOfAudioSample(); i++)
            {
                for (int j = 0; j < m_fh->getFmtHeader()->wavFormat.wChannels; j++)
                {
                    sprintf(buffer, "Time: %d - Value: %02x", i, sample_analysis[j].m_pAudioData[i].m_minAudioValue);
                    ui->textEdit->append(buffer);
                }
            }
        }
    }
}

void WaveInspector::on_actionExit_triggered()
{
    close();
}
