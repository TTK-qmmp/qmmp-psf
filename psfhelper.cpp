#include "psfhelper.h"

extern "C" {
#include "ao.h"
}

PSFHelper::PSFHelper(const QString &path)
    : m_path(path)
{
    m_info = (psf_info*)calloc(sizeof(psf_info), 1);
}

PSFHelper::~PSFHelper()
{
    deinit();
}

void PSFHelper::deinit()
{
    if(m_info) 
    {
        if(m_info->decoder && m_info->type >= 0)
        {
            ao_stop(m_info->type, m_info->decoder);
        }
        free(m_info);
    }
}

bool PSFHelper::initialize()
{
    QFile file(m_path);
    if(!file.open(QFile::ReadOnly))
    {
        qWarning("PSFHelper: open file failed");
        return false;
    }

    m_info->filesize = file.size();
    const QByteArray module = file.readAll();

    m_info->type = ao_identify((char *)module.constData());
    if(m_info->type < 0)
    {
        qWarning("PSFHelper: ao_identify error");
        return false;
    }

    m_info->decoder = ao_start(m_info->type, qPrintable(m_path), (uint8 *)module.constData(), m_info->filesize);
    if(!m_info->decoder)
    {
        qWarning("PSFHelper: ao_start error");
        return false;
    }

    ao_display_info info;
    memset(&info, 0, sizeof(info));
    bool have_info = false;
    if(ao_get_info(m_info->type, m_info->decoder, &info) == AO_SUCCESS)
    {
       have_info = true;
    }

    m_info->length = 120;
    if(!have_info)
    {
        qDebug("PSFHelper: ao has no display info");
        return true;
    }

    for(int i = 1; i < 9; i++)
    {
        if(!strncasecmp(info.title[i], "Length: ", 8))
        {
            int min;
            float sec;
            if(sscanf(info.info[i], "%d:%f", &min, &sec) == 2)
            {
                m_info->length = min * 60 + sec;
            }
            else if(sscanf(info.info[i], "%f", &sec) == 1)
            {
                m_info->length = sec;
            }
            break;
        }
    }
    return true;
}

int PSFHelper::totalTime() const
{
    return m_info->length * 1000;
}

void PSFHelper::seek(qint64 time)
{
    const int sample = time * sampleRate() / 1000;
    if(sample > m_info->currentsample)
    {
        m_info->samples_to_skip = sample - m_info->currentsample;
    }
    else
    {
        ao_command(m_info->type, m_info->decoder, COMMAND_RESTART, 0);
        m_info->samples_to_skip = sample;
    }

    m_info->currentsample = sample;
}

int PSFHelper::bitrate() const
{
    return m_info->filesize * 8.0 / totalTime();
}

int PSFHelper::sampleRate() const
{
    return 44100;
}

int PSFHelper::channels() const
{
    return 2;
}

int PSFHelper::bitsPerSample() const
{
    return 16;
}

int PSFHelper::read(unsigned char *buf, int size)
{
    if(m_info->currentsample >= m_info->length * sampleRate())
    {
        return 0;
    }

    const int initsize = size;
    while(size > 0)
    {
        if(m_info->remaining > 0)
        {
            if(m_info->samples_to_skip > 0)
            {
                int n = std::min<int>(m_info->samples_to_skip, m_info->remaining);
                if(m_info->remaining > n)
                {
                    memmove(m_info->buffer, m_info->buffer + n * 4, (m_info->remaining - n) * 4);
                }
                m_info->remaining -= n;
                m_info->samples_to_skip -= n;
                continue;
            }

            int n = size / 4;
            n = std::min<int>(m_info->remaining, n);
            memcpy(buf, m_info->buffer, n * 4);
            if(m_info->remaining > n)
            {
                memmove(m_info->buffer, m_info->buffer + n * 4, (m_info->remaining - n) * 4);
            }
            m_info->remaining -= n;
            buf += n*4;
            size -= n*4;
        }

        if(!m_info->remaining)
        {
            ao_decode(m_info->type, m_info->decoder, (int16_t *)m_info->buffer, 735);
            m_info->remaining = 735;
        }
    }

    m_info->currentsample += (initsize - size) / (channels() * bitsPerSample() / 8);
    return initsize - size;
}

QMap<Qmmp::MetaData, QString> PSFHelper::readMetaData() const
{
    QMap<Qmmp::MetaData, QString> metaData;
    if(m_info->type < 0 || !m_info->decoder)
    {
        return metaData;
    }

    ao_display_info info;
    memset(&info, 0, sizeof(info));
    bool have_info = false;
    if(ao_get_info(m_info->type, m_info->decoder, &info) == AO_SUCCESS)
    {
       have_info = true;
    }

    if(have_info)
    {
        return metaData;
    }

    for(int i = 1; i < 9; i++)
    {
        if(!strncasecmp(info.title[i], "Name: ", 6) || !strncasecmp(info.title[i], "Song: ", 6))
        {
            metaData.insert(Qmmp::TITLE, info.info[i]);
        }
        else if(!strncasecmp(info.title[i], "Game: ", 6))
        {
            metaData.insert(Qmmp::ALBUM, info.info[i]);
        }
        else if(!strncasecmp(info.title[i], "Artist: ", 8))
        {
            metaData.insert(Qmmp::ARTIST, info.info[i]);
        }
        else if(!strncasecmp(info.title[i], "Copyright: ", 11))
        {
            metaData.insert(Qmmp::COMPOSER, info.info[i]);
        }
        else if(!strncasecmp(info.title[i], "Year: ", 6))
        {
            metaData.insert(Qmmp::YEAR, info.info[i]);
        }
        else if(!strncasecmp(info.title[i], "Fade: ", 6))
        {
            // do nothing
        }
        else
        {
            char *colon = strchr(info.title[i], ':');
            char name[colon - info.title[i] + 1];
            memcpy(name, info.title[i], colon - info.title[i]);
            name[colon - info.title[i]] = 0;
            metaData.insert(Qmmp::COMMENT, info.info[i]);
        }
    }

    return metaData;
}
