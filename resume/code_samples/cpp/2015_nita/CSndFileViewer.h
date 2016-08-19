#ifndef CSNDFILEVIEWER_H
#define CSNDFILEVIEWER_H

#include "CRecSnd4FileViewer.h"

class CFileDirect;
class CGrnFileSubHeaderSound;

class QwtPlot;
class QwtPlotCurve;
class QwtPlotMarker;

/**
 * @brief The CSndFileViewer class
 *  The file viewer class for displaying information
 *  about .snd files. Displays all the data from
 *  a file: header/subheader data as a tree and
 *  two plots, one for levels, another one for
 *  samples converted to linear PCM
 */
class CSndFileViewer : public CRecSnd4FileViewer
{
    Q_OBJECT

public:
    explicit CSndFileViewer(QWidget* parent = 0);
    ~CSndFileViewer();

protected:
    bool ConstructWidgetFromFile();

    /**
     * @brief AddSndSubheaderData adds data from CGrnFileSubheaderSound struct
     *  to the data tree
     * @param subheader .snd file subheader struct
     */
    void AddSndSubheaderData(const CGrnFileSubHeaderSound& subheader);
    /**
     * @brief eventFilter this eventFilter is required to handle
     *  mouse events of the levels plot
     * @param pObject
     * @param pEvent
     * @return false in any case
     */
    bool eventFilter(QObject *pObject, QEvent *pEvent);
    /**
     * @brief setLevelMarker sets the level marker to the given time position
     *  and updates the samples plot to display the correct window around
     *  dblValue
     * @param dblValue time in seconds
     */
    void setLevelMarker(double dblValue);

    void ReadSampleData();

private slots:
    /**
     * @brief updateGraphs assigns the correct data to both the level
     *  and sample plots after the selected channel has been changed
     */
    void updateGraphs();

private:
    ///samples plot shows this amount of data in seconds
    static const unsigned int st_uSampleWindow_sec = 30;

    ///the .snd file
    CFileDirect *m_pfdFile;

    ///the position in file where the sample data starts
    unsigned long m_dwSamplesOffset;

    ///index of the currently selected channel
    unsigned int m_uCurrentChannel;

    ///number of channels from subheader
    unsigned int m_uChannelCount;
    ///mask count from subheader
    unsigned int m_uMaskCount;
    ///block step count from subheader
    unsigned int m_uBlockStepCount;

    ///number of sound blocks. Is equal to min(m_uMaskCount, grnHeader.m_uItemCount), real num of levels)
    unsigned int m_uBlockCount;

    ///levels arrays: [m_uChannelCount][m_uMaskCount]
    double** m_ppLevels;
    ///array of X coordinates for levels: time in seconds in [0, m_uMaskCount * m_uBlockCount / SOUND_FREQUENCY]
    double* m_pLevelsX;
    ///the plot widget that displays the levels plot
    QwtPlot *m_pLevelsPlot;
    QwtPlotCurve *m_pLevelsPlotData;
    QwtPlotMarker *m_pLevelsMarker;

    ///samples arrays: [m_uBlockCount], gets read from the file each time an update is needed
    double* m_pSamples;
    ///array of X coordinates for samples: time in seconds in [0, m_uMaskCount * m_uBlockCount / SOUND_FREQUENCY]
    double* m_pSamplesX;
    ///array of coding types for channels
    unsigned int* m_pCoding;
    ///samples plot widget
    QwtPlot *m_pSamplesPlot;
    QwtPlotMarker *m_pSamplesMarker;
    QwtPlotCurve *m_pSamplesPlotData;
};

#endif // CSNDFILEVIEWER_H
