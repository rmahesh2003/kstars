/*  Ekos guide tool
    Copyright (C) 2012 Jasem Mutlaq <mutlaqja@ikarustech.com>

    This application is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.
 */

#ifndef guide_H
#define guide_H

#include <QTimer>
#include <QtDBus/QtDBus>

#include "ekos/ekos.h"

#include "indi/indistd.h"
#include "indi/inditelescope.h"
#include "indi/indiccd.h"

#include "guide.h"


#include "fitsviewer/fitscommon.h"

#include "ui_guide.h"

class QTabWidget;
class FITSData;
class ScrollGraph;
class QProgressIndicator;

namespace Ekos
{

class GuideInterface;

/**
 *@class Guide
 *@short Performs calibration and autoguiding using an ST4 port or directly via the INDI driver. Can be used with the following external guiding applications:
 * PHD2
 *@author Jasem Mutlaq
 *@version 1.3
 */
class Guide : public QWidget, public Ui::Guide
{

    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.kstars.Ekos.Guide")

public:
    Guide();
    ~Guide();

    enum GuiderStage { CALIBRATION_STAGE, GUIDE_STAGE };
    enum GuiderType { GUIDE_INTERNAL, GUIDE_PHD2, GUIDE_LINGUIDER };

    /** @defgroup GuideDBusInterface Ekos DBus Interface - Capture Module
     * Ekos::Guide interface provides advanced scripting capabilities to calibrate and guide a mount via a CCD camera.
    */

    /*@{*/

    /** DBUS interface function.
     * select the CCD device from the available CCD drivers.
     * @param device The CCD device name
     * @return Returns true if CCD device is found and set, false otherwise.
     */
    Q_SCRIPTABLE bool setCCD(QString device);

    /** DBUS interface function.
     * select the ST4 device from the available ST4 drivers.
     * @param device The ST4 device name
     * @return Returns true if ST4 device is found and set, false otherwise.
     */
    Q_SCRIPTABLE bool setST4(QString device);

    /** DBUS interface function.
     * @return Returns List of registered ST4 devices.
     */
    Q_SCRIPTABLE QStringList getST4Devices();

    /**
     * @brief getStatus Return guide module status
     * @return state of guide module from Ekos::GuideState
     */
    Q_SCRIPTABLE uint getStatus() { return state;}

    /** DBUS interface function.
     * @return Returns guiding deviation from guide star in arcsecs. First elemenet is RA guiding deviation, second element is DEC guiding deviation.
     */
    Q_SCRIPTABLE QList<double> getGuidingDeviation();

    /** DBUS interface function.
     * Set CCD exposure value
     * @value exposure value in seconds.
     */
    Q_SCRIPTABLE Q_NOREPLY void setExposure(double value);

    /** DBUS interface function.
     * Set image filter to apply to the image after capture.
     * @param value Image filter (Auto Stretch, High Contrast, Equalize, High Pass)
     */
    Q_SCRIPTABLE Q_NOREPLY void setImageFilter(const QString & value);

    /** DBUS interface function.
     * Set calibration Use Two Axis option. The options must be set before starting the calibration operation. If no options are set, the options loaded from the user configuration are used.
     * @param enable if true, calibration will be performed in both RA and DEC axis. Otherwise, only RA axis will be calibrated.
     */
    Q_SCRIPTABLE Q_NOREPLY void setCalibrationTwoAxis(bool enable);

    /** DBUS interface function.
     * Set auto star calibration option. The options must be set before starting the calibration operation. If no options are set, the options loaded from the user configuration are used.
     * @param enable if true, Ekos will attempt to automatically select the best guide star and proceed with the calibration procedure.
     */
    Q_SCRIPTABLE Q_NOREPLY void setCalibrationAutoStar(bool enable);

    /** DBUS interface function.
     * In case of automatic star selection, calculate the appropriate square size given the selected star width. The options must be set before starting the calibration operation. If no options are set, the options loaded from the user configuration are used.
     * @param enable if true, Ekos will attempt to automatically select the best square size for calibration and guiding phases.
     */
    Q_SCRIPTABLE Q_NOREPLY void setCalibrationAutoSquareSize(bool enable);

    /** DBUS interface function.
     * Set calibration dark frame option. The options must be set before starting the calibration operation. If no options are set, the options loaded from the user configuration are used.
     * @param enable if true, a dark frame will be captured to subtract from the light frame.
     */
    Q_SCRIPTABLE Q_NOREPLY void setDarkFrameEnabled(bool enable);

    /** DBUS interface function.
     * Set calibration parameters.
     * @param pulseDuration Pulse duration in milliseconds to use in the calibration steps.
     */
    Q_SCRIPTABLE Q_NOREPLY void setCalibrationPulseDuration(int pulseDuration);

    /** DBUS interface function.
     * Set guiding box size. The options must be set before starting the guiding operation. If no options are set, the options loaded from the user configuration are used.
     * @param boxSizeIndex box size index (0 to 4) for box size from 8 to 128 pixels. The box size should be suitable for the size of the guide star selected. The boxSize is also used to select the subframe size around the guide star. Default is 16 pixels
     */
    Q_SCRIPTABLE Q_NOREPLY void setGuideBoxSizeIndex(int boxSizeIndex);

    /** DBUS interface function.
     * Set guiding algorithm. The options must be set before starting the guiding operation. If no options are set, the options loaded from the user configuration are used.
     * @param algorithm Select the algorithm used to calculate the centroid of the guide star (Smart, Fast, Auto, No thresh).
     */
    Q_SCRIPTABLE Q_NOREPLY void setGuideAlgorithm(const QString & algorithm);

    /** DBUS interface function.
     * Set guiding options. The options must be set before starting the guiding operation. If no options are set, the options loaded from the user configuration are used.
     * @param enable if true, it will select a subframe around the guide star depending on the boxSize size.
     */
    Q_SCRIPTABLE Q_NOREPLY void setSubFrameEnabled(bool enable);

    /** DBUS interface function.
     * Set rapid guiding option. The options must be set before starting the guiding operation. If no options are set, the options loaded from the user configuration are used.
     * @param enable if true, it will activate RapidGuide in the CCD driver. When Rapid Guide is used, no frames are sent to Ekos for analysis and the centeroid calculations are done in the CCD driver.
     */
    Q_SCRIPTABLE Q_NOREPLY void setGuideRapid(bool enable);

    /** DBUS interface function.
     * Enable or disables dithering
     * @param enable if true, dithering is enabled and is performed after each exposure is complete. Otheriese, dithering is disabled.
     * @param value dithering range in pixels. Ekos will move the guide star in a random direction for the specified dithering value in pixels.
     */
    Q_SCRIPTABLE Q_NOREPLY void setDither(bool enable, double value);


    /** DBUS interface function.
     * Selects which guiding process to utilize for calibration & guiding.
     * @param type Type of guider process to use. 0 for internal guider, 1 for external PHD2, 2 for external lin_guider
     */
    Q_SCRIPTABLE Q_NOREPLY void setGuiderType(int type);

    /** @}*/

    void addCCD(ISD::GDInterface *newCCD);
    void setTelescope(ISD::GDInterface *newTelescope);
    void addST4(ISD::ST4 *newST4);
    void setAO(ISD::ST4 *newAO);

    bool isDithering();

    void addGuideHead(ISD::GDInterface *newCCD);
    void syncTelescopeInfo();
    void syncCCDInfo();

    void clearLog();

    void setDECSwap(bool enable);
    bool sendPulse( GuideDirection ra_dir, int ra_msecs, GuideDirection dec_dir, int dec_msecs );
    bool sendPulse( GuideDirection dir, int msecs );

    QString getLogText() { return logText.join("\n"); }

    QVector3D getStarPosition() { return starCenter; }

    // Tracking Box
    void setTrackingBoxSize(int index) { boxSizeCombo->setCurrentIndex(index); }
    int getTrackingBoxSize() { return boxSizeCombo->currentText().toInt(); }

    void startRapidGuide();
    void stopRapidGuide();

public slots:

    /** DBUS interface function.
     * Start the autoguiding operation.
     * @return Returns true if guiding started successfully, false otherwise.
     */
     Q_SCRIPTABLE bool startGuiding();

    /** DBUS interface function.
     * Stop the autoguiding operation.
     * @return Returns true if guiding stopped successfully, false otherwise.
     */
     Q_SCRIPTABLE bool stopGuiding();

    /** DBUS interface function.
     * Start the calibration operation.
     * @return Returns true if calibration started successfully, false otherwise.
     */
     Q_SCRIPTABLE bool startCalibration();

    /** DBUS interface function.
     * Stop the calibration operation.
     * @return Returns true if calibration stopped successfully, false otherwise.
     */
     Q_SCRIPTABLE bool stopCalibration();

    /** DBUS interface function.
     * Capture a guide frame
     * @return Returns true if capture command is sent successfully to INDI server.
     */
     Q_SCRIPTABLE bool capture();

    /** DBUS interface function.
     * Attempts to automatically select a star from the current guide frame
     * @return Returns true if a star is selected successfully, false otherwise
     */
     Q_SCRIPTABLE bool selectAutoStar();

     void checkCCD(int ccdNum=-1);
     void checkExposureValue(ISD::CCDChip *targetChip, double exposure, IPState state);
     void newFITS(IBLOB*);
     void newST4(int index);
     void processRapidStarData(ISD::CCDChip *targetChip, double dx, double dy, double fit);
     void updateGuideDriver(double delta_ra, double delta_dec);

     // Auto Calibration Guiding (Cablirate first then start guiding immediately)
     void startAutoCalibrateGuiding();
     void checkAutoCalibrateGuiding(Ekos::GuideState state);

     void dither();
     void setSuspended(bool enable);

     void appendLogText(const QString &);

     void setStatus(Ekos::GuideState newState);

     // Star Position
     void setStarPosition(const QVector3D &newCenter, bool updateNow);

     // Capture
     void setCaptureComplete();

protected slots:
     void updateCCDBin(int index);

     /**
         * @brief processCCDNumber Process number properties arriving from CCD. Currently, binning changes are processed.
         * @param nvp pointer to number property.
         */
     void processCCDNumber(INumberVectorProperty *nvp);

     void saveDefaultGuideExposure();

     void setDefaultCCD(QString ccd);
     void setDefaultST4(QString st4);

     void updateTrackingBoxSize(int currentIndex);


     void onXscaleChanged( int i );
     void onYscaleChanged( int i );
     void onThresholdChanged( int i );
     void onInfoRateChanged( double val );
     void onEnableDirRA( bool enable );
     void onEnableDirDEC( bool enable );
     void onInputParamChanged();
     void onRapidGuideChanged(bool enable);
     void onSetDECSwap(bool enable);
     void onControlDirectionChanged(bool enable);

signals:
    void newLog();
    void newStatus(Ekos::GuideState status);

    void newStarPixmap(QPixmap &);
    void newProfilePixmap(QPixmap &);

    void newAxisDelta(double delta_ra, double delta_dec);
    void guideChipUpdated(ISD::CCDChip*);

private:
    void updateGuideParams();
    void syncTrackingBoxPosition();

    // Devices
    ISD::CCD *currentCCD;
    ISD::Telescope *currentTelescope;
    ISD::ST4* ST4Driver;
    ISD::ST4* AODriver;
    ISD::ST4* GuideDriver;

    // Device Containers
    QList<ISD::ST4*> ST4List;
    QList<ISD::CCD *> CCDs;

    // Guider process
    GuideInterface *guider;
    GuiderType guiderType;

    // Star
    QVector3D starCenter;

    // Guide Params
    double ccd_hor_pixel, ccd_ver_pixel, focal_length, aperture, guideDeviationRA, guideDeviationDEC;

    // Rapid Guide
    bool rapidGuideReticleSet;

    // State
    GuideState state;

    // Drift Graph
    ScrollGraph *driftGraph;

    // Log
    QStringList logText;

    // Misc
    bool useGuideHead;

    // Progress Activity Indicator
    QProgressIndicator *pi;
};

}

#endif  // guide_H
