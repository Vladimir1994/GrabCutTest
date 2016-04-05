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
    bool ImageRead(const string &imFileName);
    bool ImageShow();
    bool ForeGroundShow();
    bool BackGroundShow();
    void NormalizeWindowSize() const;

    string windowName() const;

    void MouseCallBack(int event, int x, int y, int flags, void *userdata);

    string savePath() const;
    bool SetSavePath(const string &p);
    bool SaveResults() const;

    bool SegmentImage();
    void gcIterCountInc();
    void gcIterCountDec();
    unsigned int gcIterCount() const;
    void Reset();
    void ChangeIsDrawMarking();

    void MarkingCircleRadiusUp();
    void MarkingCircleRadiusDown();

    GrabCutApp(const GrabCutApp &) = delete;
    GrabCutApp & operator=(const GrabCutApp &) = delete;

    static void help();

    void ChangeVoidColor();

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
    void DrawImage();
    void InitMaskByUserPoints();
    void GenResults();
    void SetBgFgMarkersToImage(Mat &img, const Scalar &fgMarker, const Scalar &bgMarker);
};

void on_mouse(int event, int x, int y, int flags, void *param);

#endif // GRABCUTAPP_H
