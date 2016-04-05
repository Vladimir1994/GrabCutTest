#include "grabcutapp.h"
#include "string"

#include "QDebug"

int main(int argc, char** argv)
{
    GrabCutApp::help();

    if (argc != 3)
        return -1;

    if (!GrabCutApp::get().ImageRead(argv[1]))
        return -1;

    GrabCutApp::get().NormalizeWindowSize();

    if (!GrabCutApp::get().SetSavePath(argv[2]))
        return -1;

    if (!GrabCutApp::get().ImageShow())
        return -1;

    setMouseCallback(GrabCutApp::get().windowName(), on_mouse, 0);

    for (;;) {
        int cmd = waitKey(0);

        if ((char)cmd == 13) {
            GrabCutApp::get().SegmentImage();
        } else if ((char)cmd == 's') {
            GrabCutApp::get().SaveResults();
        } else if ((char)cmd == 'i') {
            GrabCutApp::get().ImageShow();
        } else if ((char)cmd == 'f') {
            GrabCutApp::get().ForeGroundShow();
        } else if ((char)cmd == 'b') {
            GrabCutApp::get().BackGroundShow();
        } else if (cmd == 2490368) {
            GrabCutApp::get().gcIterCountInc();
        } else if (cmd == 2621440) {
            GrabCutApp::get().gcIterCountDec();
        } else if ((char)cmd == 'r') {
            GrabCutApp::get().Reset();
        } else if ((char)cmd == 'm') {
            GrabCutApp::get().ChangeIsDrawMarking();
        } else if ((char)cmd == 'n') {
            GrabCutApp::get().ChangeVoidColor();
        } else if ((char)cmd == 'u') {
            GrabCutApp::get().MarkingCircleRadiusUp();
        } else if ((char)cmd == 'd') {
            GrabCutApp::get().MarkingCircleRadiusDown();
        } else if ((char)cmd == 'q' or cmd == 27) {
            break;
        } else {
            if(cvGetWindowHandle(GrabCutApp::get().windowName().c_str()) == NULL) {
                break;
            }
        }
    }

    return 0;
}
