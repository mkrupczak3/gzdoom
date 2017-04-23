//
//  touch_interface.h
//  D-Touch
//
//  Created by Emile Belanger on 17/01/2016.
//  Copyright © 2016 Beloko Games. All rights reserved.
//

#ifndef touch_interface_h
#define touch_interface_h

extern "C"
{
    void mobile_init(int width, int height, const char *pngPath);

    TouchControlsInterface* mobileGetTouchInterface();
}

#endif /* touch_interface_h */
