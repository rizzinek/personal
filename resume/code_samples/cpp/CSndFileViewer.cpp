#include "CSndFileViewer.h"

//sorry I had to...
#include "../../../Recorders/RecorderSnd4/src/stdafx.h"
//required structs
#include "../../../Recorders/RecorderSnd4/src/Granite/GrnFiles.h"
//GetSoundBlockSize
#include "../../../Recorders/RecorderSnd4/src/Granite/GrnCat.h"
//for encoding algorithms
#include "../../../Recorders/RecorderSnd4/src/Granite/Common/SoundStream.h"
//file IO
#include "../../../Recorders/RecorderSnd4/src/Granite/Common/FileDirect.h"

#include <QEvent>
#include <QMouseEvent>
#include <QGridLayout>
#include <QLabel>
#include <QPen>
#include <QTreeWidgetItem>

#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_layout.h>
#include <qwt_scale_div.h>

CSndFileViewer::CSndFileViewer(QWidget* parent)
    : CRecSnd4FileViewer(parent)
    , m_pfdFile(NULL)
    , m_uCurrentChannel(0)
    , m_uChannelCount(0)
    , m_uMaskCount(0)
    , m_uBlockStepCount(0)
    , m_uBlockCount(0)
    , m_ppLevels(NULL)
    , m_pLevelsX(NULL)
    , m_pSamples(NULL)
    , m_pSamplesX(NULL)
{
    m_pLevelsPlot = new QwtPlot(this);
    m_pLevelsPlot->setAxisTitle(QwtPlot::xBottom, tr("Time, seconds"));
    m_pLevelsPlot->setAxisTitle(QwtPlot::yLeft, tr("Level"));

    m_pLevelsPlot->canvas()->installEventFilter(this);

    m_pLevelsPlotData = new QwtPlotCurve();
    m_pLevelsPlotData->attach(m_pLevelsPlot);

    m_pLevelsMarker = new QwtPlotMarker();
    m_pLevelsMarker->setLineStyle(QwtPlotMarker::VLine);
    QPen pen(Qt::blue);
    m_pLevelsMarker->setValue(0.0, 0.0);
    m_pLevelsMarker->setLinePen(pen);
    m_pLevelsMarker->attach(m_pLevelsPlot);

    const double dblMaxSampleValue = 16000.;
    m_pSamplesPlot = new QwtPlot(this);
    m_pSamplesPlot->setAxisTitle(QwtPlot::xBottom, tr("Time, seconds"));
    m_pSamplesPlot->setAxisTitle(QwtPlot::yLeft, tr("Sample"));
    //I am not sure what's the best idea for the Y scale so it is fixed for now
    m_pSamplesPlot->setAxisScale(QwtPlot::yLeft, -dblMaxSampleValue, dblMaxSampleValue);
    m_pSamplesPlot->setAxisAutoScale(QwtPlot::yLeft, false);
    m_pSamplesPlot->setAxisAutoScale(QwtPlot::xBottom, false);

    m_pSamplesPlot->canvas()->installEventFilter(this);

    m_pSamplesPlotData = new QwtPlotCurve();
    m_pSamplesPlotData->attach(m_pSamplesPlot);

    m_pSamplesMarker = new QwtPlotMarker();
    m_pSamplesMarker->setLineStyle(QwtPlotMarker::VLine);
    m_pSamplesMarker->setValue(0.0, 0.0);
    m_pSamplesMarker->setLinePen(pen);
    m_pSamplesMarker->attach(m_pSamplesPlot);

    m_pLayout->addWidget(m_pLevelsPlot, 2, 0, 1, 1);
    m_pLayout->addWidget(m_pSamplesPlot, 3, 0, 1, 1);

    m_pTreeData->setMinimumHeight(m_pSamplesPlot->height());

    connect(m_pTreeData, SIGNAL(itemSelectionChanged()), this, SLOT(updateGraphs()));
}

CSndFileViewer::~CSndFileViewer()
{
    if(m_pfdFile)
    {
        if(m_pfdFile->IsOpen())
            m_pfdFile->Close();
        delete m_pfdFile;
    }

    if(m_ppLevels)
    {
        for(unsigned int i = 0; i < m_uChannelCount; ++i)
        {
            if(m_ppLevels[i])
                delete[] m_ppLevels[i];
        }
        delete[] m_ppLevels;
        m_ppLevels = NULL;
    }

    if(m_pLevelsX)
    {
        delete[] m_pLevelsX;
        m_pLevelsX = NULL;
    }

    if(m_pSamples)
    {
        delete[] m_pSamples;
        m_pSamples = NULL;
    }

    if(m_pSamplesX)
    {
        delete[] m_pSamplesX;
        m_pSamplesX = NULL;
    }

    if(m_pCoding)
    {
        delete[] m_pCoding;
        m_pCoding = NULL;
    }
}

bool CSndFileViewer::ConstructWidgetFromFile()
{
    std::string strFileName = m_pFileNameLabel->text().toStdString();
    if(!m_pfdFile)
        m_pfdFile = new CFileDirect();
    if(m_pfdFile->IsOpen())
        m_pfdFile->Close();
    if(!m_pfdFile->Open(strFileName.c_str(), GENERIC_READ, OPEN_EXISTING))
        return false;

    //read the file header
    CGrnFileHeader grnHeader;
    if(!m_pfdFile->Read(&grnHeader, sizeof(grnHeader)))
    {
        m_pfdFile->Close();
        return false;
    }
    if(!grnHeader.Verify())
    {
        m_pfdFile->Close();
        return false;
    }
    AddHeaderData(grnHeader);

    //read the snd file subheader
    CGrnFileSubHeaderSound grnSubheader;
    if(!m_pfdFile->Read(&grnSubheader, sizeof(grnSubheader)))
    {
        m_pfdFile->Close();
        return false;
    }
    AddSndSubheaderData(grnSubheader);

    m_uChannelCount = grnSubheader.m_uChannelCount;
    m_uMaskCount = grnSubheader.m_uMaskCount;
    m_uBlockStepCount = grnSubheader.m_uBlockStepCount;

    double dblStep = static_cast<double>(m_uBlockStepCount) / SOUND_FREQUENCY;
    m_pLevelsX = new double[m_uMaskCount];
    m_pLevelsX[0] = 0.0;
    for(unsigned int i = 1; i < m_uMaskCount; ++i)
        m_pLevelsX[i] = m_pLevelsX[i - 1] + dblStep;

    m_pCoding = new unsigned int[m_uChannelCount];
    memset(m_pCoding, 0, m_uChannelCount * sizeof(unsigned int));

    //read the array of channel descriptors
    CSoundChannelDescriptor* pArrDesc = new CSoundChannelDescriptor[m_uChannelCount];
    if(!m_pfdFile->Read(pArrDesc, sizeof(*pArrDesc) * m_uChannelCount))
    {
        m_pfdFile->Close();
        return false;
    }

    QStringList qstrNameData;
    qstrNameData << tr("Channels") << QString::number(m_uChannelCount);
    QTreeWidgetItem *pChannelsItem = new QTreeWidgetItem(qstrNameData);
    qstrNameData.clear();
    m_pTreeData->addTopLevelItem(pChannelsItem);

    for(unsigned int i = 0; i < m_uChannelCount; ++i)
    {
        qstrNameData << tr("Channel ") + QString::number(pArrDesc[i].m_uID);
        QTreeWidgetItem *pChan = new QTreeWidgetItem(pChannelsItem, qstrNameData);
        pChan->setBackgroundColor(0, QColor(Qt::lightGray));
        pChan->setBackgroundColor(1, QColor(Qt::lightGray));
        pChan->setData(0, Qt::UserRole, i + 1);
        qstrNameData.clear();

        //ID
        qstrNameData << tr("ID") << QString::number(pArrDesc[i].m_uID);
        new QTreeWidgetItem(pChan, qstrNameData);
        qstrNameData.clear();

        //Coding algorithm
        qstrNameData << tr("Coding algorithm") << CodingToString(pArrDesc[i].m_uCoding);
        new QTreeWidgetItem(pChan, qstrNameData);
        qstrNameData.clear();

        m_pCoding[i] = pArrDesc[i].m_uCoding;

        //Recording mode
        qstrNameData << tr("Recording mode") << RecordingModeToString(pArrDesc[i].m_uMode);
        new QTreeWidgetItem(pChan, qstrNameData);
        qstrNameData.clear();
    }
    pChannelsItem->setExpanded(true);

    DWORD dwLvlItems = m_uChannelCount * m_uMaskCount;
    SHORT* pLvl = new SHORT[dwLvlItems];
    if(!m_pfdFile->Read(pLvl, dwLvlItems * sizeof(*pLvl)))
    {
        m_pfdFile->Close();
        return false;
    }

    //fill the levels array
    m_ppLevels = new double*[m_uChannelCount];
    for(unsigned int i = 0; i < m_uChannelCount; ++i)
    {
        m_ppLevels[i] = new double[m_uMaskCount];
        for(unsigned int j = 0; j < m_uMaskCount; ++j)
        {
            m_ppLevels[i][j] = static_cast<double>(pLvl[j * m_uChannelCount + i]);
        }
    }

    delete[] pLvl;

    m_dwSamplesOffset = m_pfdFile->GetPosition();

    int iRealNumLevels = 0;
    for(unsigned int i = 0; i < m_uChannelCount; ++i)
    {
        int iCurNumLevels = m_uMaskCount - 1;
        for(; iCurNumLevels >= 0
            && m_ppLevels[i][iCurNumLevels] == LEVEL_BLOCK_OBSOLETE
            ; --iCurNumLevels);
        if(iCurNumLevels > iRealNumLevels)
            iRealNumLevels = iCurNumLevels;
    }
    ++iRealNumLevels;
    if(m_uMaskCount != grnHeader.m_uItemCount
            || m_uMaskCount != static_cast<unsigned int>(iRealNumLevels)
            || grnHeader.m_uItemCount != static_cast<unsigned int>(iRealNumLevels))
        std::cout << "INCORRECT DATA: header's ItemCount ("
                     << grnHeader.m_uItemCount
                     << ") / subheader's MaskCount("
                     << m_uMaskCount
                     << ") / real number of nonzero levels ("
                     << iRealNumLevels <<")\n";

    m_uBlockCount = m_uMaskCount;
    if(grnHeader.m_uItemCount < m_uBlockCount)
        m_uBlockCount = grnHeader.m_uItemCount;
    if(static_cast<unsigned int>(iRealNumLevels) < m_uBlockCount)
        m_uBlockCount = iRealNumLevels;

    m_pSamplesX = new double[m_uBlockCount * m_uBlockStepCount];
    double dblXStep = 1.0 / SOUND_FREQUENCY;
    m_pSamplesX[0] = 0.0;
    for(unsigned int i = 1; i < m_uBlockCount * m_uBlockStepCount; ++i)
        m_pSamplesX[i] = m_pSamplesX[i - 1] + dblXStep;

    m_pSamples = new double[m_uBlockCount * m_uBlockStepCount];

    return true;
}

void CSndFileViewer::AddSndSubheaderData(const CGrnFileSubHeaderSound &subheader)
{
    QStringList qstrNameData;
    qstrNameData << tr("Snd subheader") << tr("Subheader of the .snd file");
    QTreeWidgetItem *pTWISubheader = new QTreeWidgetItem(qstrNameData);
    qstrNameData.clear();
    m_pTreeData->addTopLevelItem(pTWISubheader);

    //Channel count
    qstrNameData << tr("Channel count") << QString::number(subheader.m_uChannelCount);
    new QTreeWidgetItem(pTWISubheader, qstrNameData);
    qstrNameData.clear();

    //Block step count
    qstrNameData << tr("Block step count") << QString::number(subheader.m_uBlockStepCount);
    new QTreeWidgetItem(pTWISubheader, qstrNameData);
    qstrNameData.clear();

    //Mask count
    qstrNameData << tr("Mask count") << QString::number(subheader.m_uMaskCount);
    new QTreeWidgetItem(pTWISubheader, qstrNameData);
    qstrNameData.clear();
}

bool CSndFileViewer::eventFilter(QObject *pObject, QEvent *pEvent)
{
    if(pEvent->type() == QEvent::MouseButtonPress)
    {
        QMouseEvent *pMouseEvent = static_cast<QMouseEvent*>(pEvent);
        if(pObject == m_pLevelsPlot->canvas())
        {
            double dblX = m_pLevelsPlot->invTransform(QwtPlot::xBottom, pMouseEvent->pos().x());
            setLevelMarker(dblX);
        }
        else if(pObject == m_pSamplesPlot->canvas())
        {
            double dblX = m_pSamplesPlot->invTransform(QwtPlot::xBottom, pMouseEvent->pos().x());
            setLevelMarker(dblX);
        }
    }
    return false;
}

void CSndFileViewer::setLevelMarker(double dblValue)
{
    m_pLevelsMarker->setXValue(dblValue);
    m_pLevelsPlot->replot();

    if(m_uCurrentChannel > 0)
    {
        //dblValue is in seconds, need to get indices for the window
        double dblLower = dblValue - st_uSampleWindow_sec / 2;
        double dblUpper = dblValue + st_uSampleWindow_sec / 2;
        //indices of sound blocks that have to be read
        unsigned int idxLower = 0;
        unsigned int idxUpper = 0;
        if(dblLower < 0.0)
        {
            dblLower = 0.0;
            dblUpper = st_uSampleWindow_sec;
        }
        double dblFullTime = static_cast<double>(m_uMaskCount) * m_uBlockStepCount / SOUND_FREQUENCY;
        idxLower = (dblLower / dblFullTime) * m_uMaskCount;
        idxUpper = (dblUpper / dblFullTime) * m_uMaskCount;
        if(idxUpper >= m_uBlockCount)
            idxUpper = m_uBlockCount - 1;

        m_pSamplesPlotData->setRawSamples(
                    m_pSamplesX + idxLower * m_uBlockStepCount
                , m_pSamples + idxLower * m_uBlockStepCount
                , (idxUpper - idxLower + 1) * m_uBlockStepCount);
        m_pSamplesPlot->setAxisScale(
                    QwtPlot::xBottom
                    , dblLower
                    , dblUpper);
        m_pSamplesMarker->setXValue(dblValue);
        m_pSamplesPlot->replot();
    }
}

void CSndFileViewer::updateGraphs()
{
    unsigned int idx = 0;
    QTreeWidgetItem *pItem = m_pTreeData->selectedItems().first();
    while(pItem && !idx)
    {
        if(pItem->data(0, Qt::UserRole).toUInt() > 0)
            idx = pItem->data(0, Qt::UserRole).toUInt();
        else pItem = pItem->parent();
    }

    if(idx != m_uCurrentChannel)
    {
        m_uCurrentChannel = idx;
        if(idx == 0)
        {
            m_pLevelsPlot->setTitle(QString());
            m_pLevelsPlotData->setRawSamples(NULL, NULL, 0);
            m_pLevelsPlot->replot();

            m_pSamplesPlotData->setRawSamples(NULL, NULL, 0);
            m_pSamplesPlot->replot();
        }
        else
        {
            ReadSampleData();

            m_pLevelsPlot->setTitle(pItem->text(0));
            m_pLevelsPlotData->setRawSamples(m_pLevelsX, m_ppLevels[idx - 1], m_uMaskCount);
            m_pLevelsPlot->replot();

            setLevelMarker(m_pLevelsMarker->xValue());
        }
    }
}

void CSndFileViewer::ReadSampleData()
{
    unsigned int uSize = m_uBlockStepCount * m_uBlockCount;
    for(unsigned int i = 0; i < uSize; ++i)
        m_pSamples[i] = 0.0;

    BYTE* pSampleBuf = new BYTE[m_uBlockStepCount * sizeof(SHORT)];
    SHORT* pLinearBuf = new SHORT[m_uBlockStepCount];

    //offset from start of a sound block to the start of the current channel's data
    DWORD dwBlockOffset = 0;
    for(unsigned int i = 0; i < m_uCurrentChannel - 1; ++i)
        dwBlockOffset += CGrnCat::GetSoundBlockSize(m_pCoding[i], m_uBlockStepCount);

    DWORDLONG dwlCurBlockSize = CGrnCat::GetSoundBlockSize(
                m_pCoding[m_uCurrentChannel - 1], m_uBlockStepCount);

    bool bNotEOF = true;
    m_pfdFile->Seek(m_dwSamplesOffset + dwBlockOffset);
    unsigned int i = 0;
    for(; i < m_uBlockCount && bNotEOF; ++i)
    {
        memset(pLinearBuf, 0, m_uBlockStepCount * sizeof(SHORT));

        if(m_ppLevels[m_uCurrentChannel - 1][i] > LEVEL_BLOCK_OBSOLETE)
        {
            //read current block data
            bNotEOF = m_pfdFile->Read(pSampleBuf, dwlCurBlockSize);

            if(bNotEOF)
            {
                //convert it to linear pcm form
                switch(m_pCoding[m_uCurrentChannel - 1])
                {
                case CH_CODING_LINEAR:
                    memcpy(pLinearBuf, pSampleBuf, dwlCurBlockSize * sizeof(SHORT));
                    break;
                case CH_CODING_PCM:
                    gs_PCMToLinear(pLinearBuf, pSampleBuf, dwlCurBlockSize);
                    break;
                default:
                    break;
                }
            }
            else
                std::cout << "INCORRECT DATA: samples for channel "
                          << m_uCurrentChannel - 1
                          << " aren't present starting with block "
                          << i << "\n";
        }

        //fill data from linearBuf into m_pSamples
        unsigned int uCurIdxOffset = i * m_uBlockStepCount;
        for(unsigned int j = 0; j < m_uBlockStepCount; ++j)
        {
            m_pSamples[uCurIdxOffset + j] = pLinearBuf[j];
        }

        if(i < m_uBlockCount - 1 && bNotEOF)
        {
            dwBlockOffset = 0;
            for(unsigned int j = m_uCurrentChannel; j < m_uChannelCount; ++j)
                if(m_ppLevels[j][i] > LEVEL_BLOCK_OBSOLETE)
                    dwBlockOffset += CGrnCat::GetSoundBlockSize(m_pCoding[j], m_uBlockStepCount);
            for(unsigned int j = 0; j < m_uCurrentChannel - 1; ++j)
                if(m_ppLevels[j][i + 1] > LEVEL_BLOCK_OBSOLETE)
                    dwBlockOffset += CGrnCat::GetSoundBlockSize(m_pCoding[j], m_uBlockStepCount);
            //move to the next block of this channel's data in the file
            m_pfdFile->Seek(m_pfdFile->GetPosition() + dwBlockOffset);
        }
    }

    delete[] pSampleBuf;
    delete[] pLinearBuf;
}
