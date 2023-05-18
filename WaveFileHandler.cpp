#include "WaveFileHandler.h"
#include <QByteArray>
//#include "waveinspector.h"

const unsigned int STANDARD_WAVEFORMAT_SIZE = 16;
const unsigned int BITS_PER_BYTE = 8;

CWaveFileHandler::CWaveFileHandler(void): m_wavfile(0), m_pRiffHeader(0), m_pFmtHeader(0),
                                          m_pDataBlock(0), m_pAudioChannels(0)
{
}

CWaveFileHandler::~CWaveFileHandler(void)
{
    if (m_pAudioChannels != 0)
    {
        if (m_pFmtHeader != 0)
        {
            for (int i=0; i < m_pFmtHeader->wavFormat.wChannels; i++ )
            {
                if (m_pAudioChannels[i].m_pAudioData != 0)
                {
                    delete [] (m_pAudioChannels[i].m_pAudioData);
                    m_pAudioChannels[i].m_pAudioData = 0;
                }
            }
        }
        delete [] m_pAudioChannels;
        m_pAudioChannels = 0;
    }

    if (m_wavfile != 0)
    {
        m_wavfile->close();
        delete m_wavfile;
        m_wavfile = 0;
    }
    if (m_pRiffHeader != 0)
    {
        delete m_pRiffHeader;
        m_pRiffHeader = 0;
    }
    if (m_pFmtHeader != 0)
    {
        delete m_pFmtHeader;
        m_pFmtHeader = 0;
    }
    if (m_pDataBlock != 0)
    {
        delete m_pDataBlock;
        m_pDataBlock = 0;
    }
}

// Opens a wav file for reading. fails if not exists. throws char * exception
bool CWaveFileHandler::OpenForRead(const char* szFile)
{
   bool ret = false;

   m_wavfile = new QFile(szFile);
   if (m_wavfile != 0)
   {
       ret = m_wavfile->open(QIODevice::ReadOnly);
   }
   else
   {
       ret = false;
   }
   return ret;
}

bool CWaveFileHandler::isValidRiffHeader()
{
    bool ret = false;

    QByteArray chunk = m_wavfile->read(12);
    if(!(	(chunk.at(0) == 'R') &&
            (chunk.at(1) == 'I') &&
            (chunk.at(2) == 'F') &&
            (chunk.at(3) == 'F') ) )
    {
        ret = false;
    }
    else
    {
        if(!(	(chunk.at(8) == 'W') &&
                (chunk.at(9) == 'A') &&
                (chunk.at(10) == 'V') &&
                (chunk.at(11) == 'E') ) )
        {
            ret = false;
        }
        else
        {
            m_pRiffHeader = new RIFF_HEADER;
            m_pRiffHeader->szRiffID[0] = chunk.at(0);
            m_pRiffHeader->szRiffID[1] = chunk.at(1);
            m_pRiffHeader->szRiffID[2] = chunk.at(2);
            m_pRiffHeader->szRiffID[3] = chunk.at(3);

            uint32_t store = 0;
            memset((void *)&m_pRiffHeader->dwRiffSize, 0, sizeof(uint32_t));
            m_pRiffHeader->dwRiffSize = (uint8_t) chunk.at(7);
            m_pRiffHeader->dwRiffSize <<= BITS_PER_BYTE * 3;
            store = (uint8_t) chunk.at(6) << BITS_PER_BYTE * 2;
            m_pRiffHeader->dwRiffSize |= store;
            store = (uint8_t) chunk.at(5) << BITS_PER_BYTE;
            m_pRiffHeader->dwRiffSize |= store;
            m_pRiffHeader->dwRiffSize |= (uint8_t) chunk.at(4);

            m_pRiffHeader->szRiffFormat[0] = chunk.at(8);
            m_pRiffHeader->szRiffFormat[1] = chunk.at(9);
            m_pRiffHeader->szRiffFormat[2] = chunk.at(10);
            m_pRiffHeader->szRiffFormat[3] = chunk.at(11);
            ret = true;
        }
    }
    return ret;
}

// verifies fmt chunk. checks for 'fmt '
qint64 CWaveFileHandler::FindValidFmtChunk()
{
    qint64 ret = -1;
    bool carryOn = true;
    qint64 foundAt = -1;
    qint64 pos = 12; //RIFF already read
    QByteArray charRead(1025, '\0');

    /**
      * Look for the fmt block that coulb be anywhere although it is normally just after the RIFF
      */
    while (carryOn == true)
    {
        charRead = m_wavfile->read(1024);
        if (charRead.size() > 0)
        {
            foundAt = charRead.indexOf("fmt ");
            if (foundAt >= 0)
            {
                pos+=foundAt;
                /**
                 * Position the pointer at the beginning of the chunk
                 */
                m_wavfile->seek(pos);
                /**
                 * Read ID and length of the block
                 */
                charRead = m_wavfile->read(8);
                m_pFmtHeader = new FMT_BLOCK;
                m_pFmtHeader->szFmtID[0] = charRead.at(0);
                m_pFmtHeader->szFmtID[1] = charRead.at(1);
                m_pFmtHeader->szFmtID[2] = charRead.at(2);
                m_pFmtHeader->szFmtID[3] = charRead.at(3);
                uint32_t store = 0;
                memset((void *)&m_pFmtHeader->dwFmtSize, 0, sizeof(uint32_t));
                m_pFmtHeader->dwFmtSize = (uint8_t) charRead.at(7);
                m_pFmtHeader->dwFmtSize <<= BITS_PER_BYTE * 3;
                store = (uint8_t) charRead.at(6) << BITS_PER_BYTE * 2;
                m_pFmtHeader->dwFmtSize |= store;
                store = (uint8_t) charRead.at(5) << BITS_PER_BYTE;
                m_pFmtHeader->dwFmtSize |= store;
                m_pFmtHeader->dwFmtSize |= (uint8_t) charRead.at(4);
                /**
                 * Read the rest of the fmt block
                 */
                /**
                 * Format: UNCOMPRESSED or COMPRESSED (not handled)
                 */
                charRead = m_wavfile->read(m_pFmtHeader->dwFmtSize);
                memset((void *)&m_pFmtHeader->wavFormat.wFormatTag, 0, sizeof(uint16_t));
                m_pFmtHeader->wavFormat.wFormatTag = (uint8_t) charRead.at(1);
                m_pFmtHeader->wavFormat.wFormatTag <<= BITS_PER_BYTE;
                m_pFmtHeader->wavFormat.wFormatTag |= (uint8_t) charRead.at(0);
                if (m_pFmtHeader->wavFormat.wFormatTag == 1)
                {
                    /**
                     * No of channels
                     */
                    memset((void *)&m_pFmtHeader->wavFormat.wChannels, 0, sizeof(uint16_t));
                    m_pFmtHeader->wavFormat.wChannels = (uint8_t) charRead.at(3);
                    m_pFmtHeader->wavFormat.wChannels <<= BITS_PER_BYTE;
                    m_pFmtHeader->wavFormat.wChannels |= (uint8_t) charRead.at(2);
                    /**
                     * Samples per Second
                     */
                    store = 0;
                    memset((void *)&m_pFmtHeader->wavFormat.dwSamplesPerSec, 0, sizeof(uint32_t));
                    m_pFmtHeader->wavFormat.dwSamplesPerSec = (uint8_t) charRead.at(7);
                    m_pFmtHeader->wavFormat.dwSamplesPerSec <<= BITS_PER_BYTE * 3;
                    store = (uint8_t) charRead.at(6) << BITS_PER_BYTE * 2;
                    m_pFmtHeader->wavFormat.dwSamplesPerSec |= store;
                    store = (uint8_t) charRead.at(5) << BITS_PER_BYTE;
                    m_pFmtHeader->wavFormat.dwSamplesPerSec |= store;
                    m_pFmtHeader->wavFormat.dwSamplesPerSec |= (uint8_t) charRead.at(4);
                    /**
                     * Average Bytes per Second
                     */
                    store = 0;
                    memset((void *)&m_pFmtHeader->wavFormat.dwAvgBytesPerSec, 0, sizeof(uint32_t));
                    m_pFmtHeader->wavFormat.dwAvgBytesPerSec = (uint8_t) charRead.at(11);
                    m_pFmtHeader->wavFormat.dwAvgBytesPerSec <<= BITS_PER_BYTE * 3;
                    store = (uint8_t) charRead.at(10) << BITS_PER_BYTE * 2;
                    m_pFmtHeader->wavFormat.dwAvgBytesPerSec |= store;
                    store = (uint8_t) charRead.at(9) << BITS_PER_BYTE;
                    m_pFmtHeader->wavFormat.dwAvgBytesPerSec |= store;
                    m_pFmtHeader->wavFormat.dwAvgBytesPerSec |= (uint8_t) charRead.at(8);
                    /**
                     * Block Align
                     */
                    memset((void *)&m_pFmtHeader->wavFormat.wBlockAlign, 0, sizeof(uint16_t));
                    m_pFmtHeader->wavFormat.wBlockAlign = (uint8_t) charRead.at(13);
                    m_pFmtHeader->wavFormat.wBlockAlign <<= BITS_PER_BYTE;
                    m_pFmtHeader->wavFormat.wBlockAlign |= (uint8_t) charRead.at(12);
                    /**
                     * Bits per Sample
                     */
                    memset((void *)&m_pFmtHeader->wavFormat.wBitsPerSample, 0, sizeof(uint16_t));
                    m_pFmtHeader->wavFormat.wBitsPerSample = (uint8_t) charRead.at(15);
                    m_pFmtHeader->wavFormat.wBitsPerSample <<= BITS_PER_BYTE;
                    m_pFmtHeader->wavFormat.wBitsPerSample |= (uint8_t) charRead.at(14);
                }
                else
                {
                    /**
                     * Only uncompressed files are managed by the application
                     * Flag set as if the fmt block had not been found
                     */
                    foundAt = -1;
                }
                carryOn = false;
            }
            else
            {
                pos+=1024;
            }
        }
        else
        {
            carryOn = false;
        }
    }
    if (foundAt >= 0)
    {
        ret = pos;
    }
    else
    {
        ret = -1;
    }
    return ret;
}

// checks for 'data' 
qint64 CWaveFileHandler::FindValidDataChunk()
{
    qint64 ret = -1;
    bool carryOn = true;
    qint64 foundAt = -1;
    qint64 pos = 12; //RIFF already read
    QByteArray charRead(1025, '\0');

    /**
      * Look for the data block that could be anywhere although it must be after the FMT_BLOCK.
      * Start search from the beginning (no check that data block comes after fmt for now)
      */
    m_wavfile->seek(0);
    while (carryOn == true)
    {
        charRead = m_wavfile->read(1024);
        if (charRead.size() > 0)
        {
            foundAt = charRead.indexOf("data");
            if (foundAt >= 0)
            {
                pos = foundAt;
                /**
                 * Position the pointer at the beginning of the chunk
                 */
                m_wavfile->seek(pos);
                /**
                 * Read ID and length of the block
                 */
                charRead = m_wavfile->read(8);
                m_pDataBlock = new DATA_BLOCK;
                m_pDataBlock->szDataID[0] = charRead.at(0);
                m_pDataBlock->szDataID[1] = charRead.at(1);
                m_pDataBlock->szDataID[2] = charRead.at(2);
                m_pDataBlock->szDataID[3] = charRead.at(3);
                uint32_t store = 0;
                memset((void *)&m_pDataBlock->dwDataSize, 0, sizeof(uint32_t));
                m_pDataBlock->dwDataSize = (uint8_t) charRead.at(7);
                m_pDataBlock->dwDataSize <<= BITS_PER_BYTE * 3;
                store = (uint8_t) charRead.at(6) << BITS_PER_BYTE * 2;
                m_pDataBlock->dwDataSize |= store;
                store = (uint8_t) charRead.at(5) << BITS_PER_BYTE;
                m_pDataBlock->dwDataSize |= store;
                m_pDataBlock->dwDataSize |= (uint8_t) charRead.at(4);
                carryOn = false;
            }
            else
            {
                pos+=1024;
            }
        }
        else
        {
            carryOn = false;
        }
    }
    if (foundAt >= 0)
    {
        ret = pos;
    }
    else
    {
        ret = -1;
    }
    return ret;
}


void CWaveFileHandler::analyzedataChunk()
{
    int32_t filePointer = 0;
    int16_t secNo = 0;
    int32_t dataChunkSize = m_pDataBlock->dwDataSize;
    uint32_t oneSecondMusicSize = m_pFmtHeader->wavFormat.dwSamplesPerSec * m_pFmtHeader->wavFormat.wChannels;
    uint16_t current_channel = 0;
    uint16_t samplesize_byte = 0;

    QByteArray charRead(oneSecondMusicSize + 1, '\0');
    m_pAudioChannels = new AUDIO_CHANNEL[m_pFmtHeader->wavFormat.wChannels];
    for(int i= 0; i < m_pFmtHeader->wavFormat.wChannels; i++)
    {
        // +1 because the value returned by the function is truncated
        m_pAudioChannels[i].m_pAudioData = new AUDIO_DATA[lengthOfAudioSample()+1];
        for(int j=0; j < lengthOfAudioSample()+1; j++)
        {
            m_pAudioChannels[i].m_pAudioData[j].m_minAudioValue = 0;
            m_pAudioChannels[i].m_pAudioData[j].m_maxAudioValue = 0;
        }
    }
    while (filePointer < dataChunkSize)
    {
        charRead = m_wavfile->read(oneSecondMusicSize);
        current_channel = 0;
        while(current_channel < m_pFmtHeader->wavFormat.wChannels)
        {
            uint8_t * channel_sample = new uint8_t[m_pFmtHeader->wavFormat.wBitsPerSample/BITS_PER_BYTE];
            samplesize_byte = 0;
            while (samplesize_byte < m_pFmtHeader->wavFormat.wBitsPerSample/BITS_PER_BYTE)
            {
                uint16_t bufPointer = current_channel*(m_pFmtHeader->wavFormat.wBitsPerSample/BITS_PER_BYTE) + samplesize_byte;
                channel_sample[samplesize_byte] = charRead[bufPointer];
                samplesize_byte++;
            }
            m_pAudioChannels[current_channel].m_pAudioData[secNo].m_minAudioValue = channel_sample[1];
            m_pAudioChannels[current_channel].m_pAudioData[secNo].m_minAudioValue <<= BITS_PER_BYTE;
            m_pAudioChannels[current_channel].m_pAudioData[secNo].m_minAudioValue |= channel_sample[0];
            delete [] channel_sample;
            current_channel++;
        }
        filePointer += oneSecondMusicSize;
        secNo++;
    }
}

uint16_t CWaveFileHandler::lengthOfAudioSample()
{
    uint16_t noOfSeconds = m_pDataBlock->dwDataSize/(m_pFmtHeader->wavFormat.dwSamplesPerSec*(m_pFmtHeader->wavFormat.wBitsPerSample/8)* m_pFmtHeader->wavFormat.wChannels);
    return noOfSeconds;
}


// closes the file that was opened for reading
void CWaveFileHandler::CloseReader(void)
{
}
