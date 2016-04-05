#include "grabcutapp.h"

#include <QString>
#include <QStringList>
#include <QDebug>
#include <QDir>

#include <algorithm>

#include <math.h>

GrabCutApp::GrabCutApp() : windowName_("GrabCut"), imageShowWidth_(1000),
    imageLoaded_(false), rectState_(NOT_SET), leftButtonPressed_(false),
    rightButtonPressed_(false), mouseMoveCounter_(mouseMoveCounterPeriod_),
    mouseMoveCounterPeriod_(4), markingLineWidth_(5), maskSet_(false), gcIterCount_(1),
    userMaskSet_(false), markingCircleRadius_(4), markingCircleRadiusChangeStep_(2),
    isDrawMarking_(true), voidColor_(Scalar(0, 255, 0)), currentVoidColorIdx_(0)
{
    namedWindow(windowName_, WINDOW_NORMAL);

    image_ = make_shared<Mat> ();
    fgResult_ = make_shared<Mat> ();
    bgResult_ = make_shared<Mat> ();

    voidColors_.append(Scalar(0, 255, 0));
    voidColors_.append(Scalar(0, 0, 0));
    voidColors_.append(Scalar(255, 255, 255));
}

GrabCutApp & GrabCutApp::get()
{
    static GrabCutApp instance;
    return instance;
}

GrabCutApp::~GrabCutApp()
{
    destroyWindow(windowName_);
}

bool GrabCutApp::imageRead(const string &imFileName)
{
    imFileName_ = imFileName;

    if (imFileName_.empty()) {
        cerr << "image filename could not be empty" << endl;
        return false;
    }

    QString qImFileName = QString(imFileName_.c_str());
    QStringList qImFileNameList = qImFileName.split("\\");
    qImFileName = qImFileNameList.join("\\\\");
    imFileName_ = qImFileName.toStdString();
    *image_ = imread(imFileName_, CV_LOAD_IMAGE_COLOR);

    Size imSize = image_->size();
    if (imSize.height > 2500 || imSize.width > 2500) {
        cerr << "image is too big" << endl;
        return false;
    }

    if (image_->empty()) {
        cerr << "could not open or find the image" << endl ;
        return false;
    }

    imageLoaded_ = true;
    return true;
}

bool GrabCutApp::imageShow()
{
    if (imageLoaded_) {
        activeImage_ = image_;
        drawImage();
        cout << "show image" << endl;
        return true;
    }
    else {
        cerr <<  "image is not loaded" << endl;
        return false;
    }
}

bool GrabCutApp::foreGroundShow()
{
    if (!cutDone_)
        return false;
    activeImage_ = fgResult_;
    drawImage();
    cout << "show foreground" << endl;
    return true;
}

bool GrabCutApp::backGroundShow()
{
    if (!cutDone_)
        return false;
    activeImage_ = bgResult_;
    drawImage();
    cout << "show background" << endl;
    return true;
}

void GrabCutApp::normalizeWindowSize() const
{
    Size imSize = image_->size();
    if ((unsigned int)imSize.width > imageShowWidth_) {
        int newImHeight = imageShowWidth_
            * ((float)imSize.height / (float)imSize.width);
        resizeWindow(windowName_, imageShowWidth_, newImHeight);
    }
}

string GrabCutApp::windowName() const
{
    return windowName_;
}

void GrabCutApp::mouseCallBack(int event, int x, int y, int flags, void *)
{
    if (!isDrawMarking_)
        return;

    if (!(flags & CV_EVENT_FLAG_CTRLKEY)) {
        if (event == EVENT_LBUTTONDOWN) {
            leftButtonPressed_ = true;
            firstMousePoint_.x = x;
            firstMousePoint_.y = y;
            secondMousePoint_.x = x;
            secondMousePoint_.y = y;

            rectState_ = IN_PROGRESS;

            drawImage();
        } else if (event == EVENT_LBUTTONUP) {
            Size imSize = image_->size();
            secondMousePoint_.x = max(0, secondMousePoint_.x);
            secondMousePoint_.y = max(0, secondMousePoint_.y);
            secondMousePoint_.x = min(imSize.width, secondMousePoint_.x);
            secondMousePoint_.y = min(imSize.height, secondMousePoint_.y);

            markingRect_.x = min(firstMousePoint_.x, secondMousePoint_.x);
            markingRect_.y = min(firstMousePoint_.y, secondMousePoint_.y);
            markingRect_.width = abs(firstMousePoint_.x - secondMousePoint_.x);
            markingRect_.height = abs(firstMousePoint_.y - secondMousePoint_.y);

            rectState_ = SET;

            leftButtonPressed_ = false;
        } else if (event == EVENT_MOUSEMOVE) {
            if (leftButtonPressed_) {
                if(mouseMoveCounter_) {
                    mouseMoveCounter_--;
                } else {
                    secondMousePoint_.x = x;
                    secondMousePoint_.y = y;
                    mouseMoveCounter_ = mouseMoveCounterPeriod_;
                    drawImage();
                }
            }
        } else if (event == EVENT_RBUTTONDOWN) {
            rectState_ = NOT_SET;
            drawImage();
        }
    } else {
        if (event == EVENT_LBUTTONDOWN) {
            leftButtonPressed_ = true;
            userMaskSet_ = true;
            fgPixels_.append(Point3i(x, y, markingCircleRadius_));
            drawImage();
        } else if (event == EVENT_RBUTTONDOWN) {
            rightButtonPressed_ = true;
            userMaskSet_ = true;
            bgPixels_.append(Point3i(x, y, markingCircleRadius_));
            drawImage();
        } else if (event == EVENT_LBUTTONUP) {
            leftButtonPressed_ = false;
        } else if (event == EVENT_RBUTTONUP) {
            rightButtonPressed_ = false;
        } else if (event == EVENT_MOUSEMOVE) {
            if (mouseMoveCounter_) {
                mouseMoveCounter_--;
            } else {
                if (leftButtonPressed_) {
                    fgPixels_.append(Point3i(x, y, markingCircleRadius_));
                    mouseMoveCounter_ = mouseMoveCounterPeriod_;
                    drawImage();
                } else if (rightButtonPressed_) {
                    bgPixels_.append(Point3i(x, y, markingCircleRadius_));
                    mouseMoveCounter_ = mouseMoveCounterPeriod_;
                    drawImage();
                }
            }
        }
    }
}

string GrabCutApp::savePath() const
{
    return savePath_;
}

bool GrabCutApp::setSavePath(const string & p)
{
    QDir dir;
    dir.mkpath(QString(p.c_str()));
    dir.setPath(QString(p.c_str()));
    if (!dir.exists()) {
        cerr << "save directory does not exist and it is impossible to create" << endl;
        return false;
    }
    savePath_ = p;
    return true;
}

bool GrabCutApp::saveResults() const
{
    if (!cutDone_) {
        cerr << "nothing to save" << endl;
        return false;
    }

    QString qImFileName = QString(imFileName_.c_str());
    QStringList qImFileNameList = qImFileName.split("\\");
    string qImName = qImFileNameList.last().split(".").first().toStdString();
    if (imwrite(savePath_ + "\\" + qImName + "_foreground.jpg", * fgResult_)
            && imwrite(savePath_ + "\\" + qImName + "_background.jpg", * bgResult_)) {
        cout << "save done to " << savePath_ << endl;
        return true;
    } else {
        cerr << "save failed" << endl;
        return false;
    }
}

bool GrabCutApp::segmentImage()
{
    cout << "start GrabCut" << endl;

    if (rectState_ != SET) {
        cerr << "marking rectangle was not set" << endl;
        return false;
    }

    if (userMaskSet_)
        initMaskByUserPoints();

    if (!maskSet_) {
        grabCut(*image_, mask_, markingRect_, bgModel_,
                fgModel_, gcIterCount_, GC_INIT_WITH_RECT);
        cout << "GrabCut done using rectangle" << endl;
    } else {
        grabCut(*image_, mask_, markingRect_,
                bgModel_, fgModel_, gcIterCount_);
        cout << "GrabCut done using rectangle and mask" << endl;
    }

    maskSet_ = true;
    genResults();

    cutDone_ = true;
    foreGroundShow();
    return true;
}

void GrabCutApp::gcIterCountInc()
{
    gcIterCount_++;
    cout << "GrabCut iteration count " << gcIterCount_ << endl;
}

void GrabCutApp::gcIterCountDec()
{
    if (gcIterCount_ > 1) {
        gcIterCount_--;
        cout << "GrabCut iteration count " << gcIterCount_ << endl;
    } else {
        cerr << "GrabCut iteration count " << gcIterCount_ << endl;
        cerr << "impossible to set iteration count less than one" << endl;
    }
}

unsigned int GrabCutApp::gcIterCount() const
{
    return gcIterCount_;
}

void GrabCutApp::reset()
{
    cutDone_ = false;
    gcIterCount_ = 1;
    rectState_ = NOT_SET;
    maskSet_ = false;
    userMaskSet_ = false;
    if (!mask_.empty())
        mask_.setTo(Scalar::all(GC_BGD));
    bgPixels_.clear();
    fgPixels_.clear();
    activeImage_ = image_;
    drawImage();
    cout << "reset" << endl;
}

void GrabCutApp::changeIsDrawMarking()
{
    isDrawMarking_ = !isDrawMarking_;
    if (isDrawMarking_)
        cout << "drawing markring on" << endl;
    else
        cout << "drawing markring off" << endl;
    drawImage();
}

void GrabCutApp::markingCircleRadiusUp()
{
    markingCircleRadius_ += markingCircleRadiusChangeStep_;
    cout << "marking circle radius is" << markingCircleRadius_ << endl;
}

void GrabCutApp::markingCircleRadiusDown()
{
    if (markingCircleRadius_ > 0) {
        markingCircleRadius_ -= markingCircleRadiusChangeStep_;
        cout << "marking circle radius is " << markingCircleRadius_ << endl;
    } else {
        cerr << "unable to set marking circle radius less than 0" << endl;
    }
}

void GrabCutApp::help()
{
    cout << "\nThis program demonstrates GrabCut segmentation -- select an object in a \n"
            "rectangle and then grabcut will attempt to segment it out.\n"
            "Call:\n"
            "./GrabCutTest <image_name> <save_path>\n"
            "\nSelect a rectangular area around the object you want to segment\n" <<
            "\nHot keys: \n"
            "\tESC or q - quit the program\n"
            "\tr - reset segmentation and start again\n"
            "\tEnter - make GrabCut\n"
            "\n"
            "\tleft mouse button - set rectangle\n"
            "\n"
            "\tCTRL+left mouse button - set foreground pixels\n"
            "\tCTRL+right mouse button - set background pixels\n"
            "\n"
            "\ts - save background and foreground to the save path\n"
            "\n"
            "\ti - show image\n"
            "\tf - show foreground\n"
            "\tb - show background\n"
            "\n"
            "\tm - on\\off marking\n"
            "\tn - change void color green\\black\\white\n"
            "\n"
            "\tup - increment GrabCut iteration count parameter\n"
            "\tdown - decrement GrabCut iteration count parameter\n"
            "\n"
            "\tu - increase circle marking radius\n"
            "\td - decrease circle marking radius\n" << endl;
}

void GrabCutApp::changeVoidColor()
{
    if (currentVoidColorIdx_ < (unsigned int)voidColors_.size() - 1)
        currentVoidColorIdx_++;
    else
        currentVoidColorIdx_ = 0;
    voidColor_ = voidColors_[currentVoidColorIdx_];

    cout << "void color is " << voidColor_ << endl;

    if (cutDone_) {
        genResults();
        drawImage();
    }
}

void GrabCutApp::drawImage()
{
    if ((rectState_ == NOT_SET && !userMaskSet_) || !isDrawMarking_) {
        imshow(windowName_, * activeImage_);
        return;
    }

    Mat imgCopy;
    activeImage_->copyTo(imgCopy);
    if (rectState_ != NOT_SET)
        rectangle(imgCopy, firstMousePoint_, secondMousePoint_,
                  Scalar(0, 255, 255), markingLineWidth_);
    if (userMaskSet_)
        setBgFgMarkersToImage(imgCopy, Scalar(255, 0, 0), Scalar(0, 0, 255));
    imshow(windowName_, imgCopy);
}

void GrabCutApp::initMaskByUserPoints()
{
    if (mask_.empty()) {
        Size s = image_->size();
        mask_.create(s, CV_8UC1);
        mask_.setTo(Scalar::all(GC_BGD));
    }    
    mask_(markingRect_).setTo(Scalar::all(GC_PR_FGD));
    setBgFgMarkersToImage(mask_, Scalar(GC_FGD), Scalar(GC_BGD));
    maskSet_ = true;
}

void GrabCutApp::genResults()
{
    Mat mask;
    Mat maskFg;
    Mat maskPrFg;
    compare(mask_, GC_PR_FGD, maskPrFg, CMP_EQ);
    compare(mask_, GC_FGD, maskFg, CMP_EQ);
    mask = maskPrFg + maskFg;

    Size s = image_->size();
    if (fgResult_->empty())
        fgResult_->create(s, CV_8UC3);
    fgResult_->setTo(voidColor_);

    if (bgResult_->empty())
        bgResult_->create(s, CV_8UC3);
    bgResult_->setTo(voidColor_);

    image_->copyTo(*fgResult_, mask);
    image_->copyTo(*bgResult_, Scalar(255,255,255) - mask);
}

void GrabCutApp::setBgFgMarkersToImage(Mat &img, const Scalar &fgMarker,
    const Scalar &bgMarker)
{
    Point p;
    foreach (auto it, fgPixels_) {
        p.x = it.x;
        p.y = it.y;
        circle(img, p, it.z, fgMarker, -1);
    }
    foreach (auto it, bgPixels_) {
        p.x = it.x;
        p.y = it.y;
        circle(img, p, it.z, bgMarker, -1);
    }
}

void onMouse(int event, int x, int y, int flags, void *param)
{
    GrabCutApp::get().mouseCallBack(event, x, y, flags, param);
}
