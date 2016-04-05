#include "grabcutapp.h"
#include "string"

#include "QDebug"

int main(int argc, char** argv)
{
    GrabCutApp::help();

    if (argc != 3)
        return -1;

    if (!GrabCutApp::get().imageRead(argv[1]))
        return -1;

    GrabCutApp::get().normalizeWindowSize();

    if (!GrabCutApp::get().setSavePath(argv[2]))
        return -1;

    if (!GrabCutApp::get().imageShow())
        return -1;

    setMouseCallback(GrabCutApp::get().windowName(), onMouse, 0);

    for (;;) {
        int cmd = waitKey(0);

        if ((char)cmd == 13) {
            GrabCutApp::get().segmentImage();
        } else if ((char)cmd == 's') {
            GrabCutApp::get().saveResults();
        } else if ((char)cmd == 'i') {
            GrabCutApp::get().imageShow();
        } else if ((char)cmd == 'f') {
            GrabCutApp::get().foreGroundShow();
        } else if ((char)cmd == 'b') {
            GrabCutApp::get().backGroundShow();
        } else if (cmd == 2490368) {
            GrabCutApp::get().gcIterCountInc();
        } else if (cmd == 2621440) {
            GrabCutApp::get().gcIterCountDec();
        } else if ((char)cmd == 'r') {
            GrabCutApp::get().reset();
        } else if ((char)cmd == 'm') {
            GrabCutApp::get().changeIsDrawMarking();
        } else if ((char)cmd == 'n') {
            GrabCutApp::get().changeVoidColor();
        } else if ((char)cmd == 'u') {
            GrabCutApp::get().markingCircleRadiusUp();
        } else if ((char)cmd == 'd') {
            GrabCutApp::get().markingCircleRadiusDown();
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
