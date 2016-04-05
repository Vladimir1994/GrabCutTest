#ifndef GRABCUTAPP_H
#define GRABCUTAPP_H

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <QVector>
#include <QPoint>

#include <string>
#include <iostream>
#include <cctype>
#include <memory>

using namespace cv;
using namespace std;

class GrabCutApp
{
public:
    static GrabCutApp & get();
    ~GrabCutApp();
    bool imageRead(const string &imFileName);
    bool imageShow();
    bool foreGroundShow();
    bool backGroundShow();
    void normalizeWindowSize() const;

    string windowName() const;

    void mouseCallBack(int event, int x, int y, int flags, void *userdata);

    string savePath() const;
    bool setSavePath(const string &p);
    bool saveResults() const;

    bool segmentImage();
    void gcIterCountInc();
    void gcIterCountDec();
    unsigned int gcIterCount() const;
    void reset();
    void changeIsDrawMarking();

    void markingCircleRadiusUp();
    void markingCircleRadiusDown();

    GrabCutApp(const GrabCutApp &) = delete;
    GrabCutApp & operator=(const GrabCutApp &) = delete;

    static void help();

    void changeVoidColor();

private:   
    string windowName_;

    shared_ptr<Mat> activeImage_;
    shared_ptr<Mat> image_;
    const size_t imageShowWidth_;
    bool imageLoaded_;
    string imFileName_;
    string savePath_;

    Point firstMousePoint_;
    Point secondMousePoint_;
    Rect markingRect_;

    enum rectStateType {NOT_SET, IN_PROGRESS, SET};

    rectStateType rectState_;
    bool leftButtonPressed_;
    bool rightButtonPressed_;
    unsigned int mouseMoveCounter_;
    const unsigned int mouseMoveCounterPeriod_;
    const size_t markingLineWidth_;

    shared_ptr<Mat> fgResult_;
    shared_ptr<Mat> bgResult_;
    bool cutDone_;
    Mat fgModel_;
    Mat bgModel_;
    Mat mask_;
    bool maskSet_;
    unsigned int gcIterCount_;

    QVector<Point3i> fgPixels_;
    QVector<Point3i> bgPixels_;
    bool userMaskSet_;
    size_t markingCircleRadius_;
    size_t markingCircleRadiusChangeStep_;

    bool isDrawMarking_;

    Scalar voidColor_;
    unsigned int currentVoidColorIdx_;
    QVector<Scalar> voidColors_;

private:
    explicit GrabCutApp();
    void drawImage();
    void initMaskByUserPoints();
    void genResults();
    void setBgFgMarkersToImage(Mat &img, const Scalar &fgMarker, const Scalar &bgMarker);
};

void onMouse(int event, int x, int y, int flags, void *param);

#endif // GRABCUTAPP_H
