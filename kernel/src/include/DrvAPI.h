////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                     Copyright (C) 2023, Joey Qytyku                        //
//                                                                            //
// This file is part of OS/90 and is published under the GNU General Public   //
// License version 2. A copy of this license should be included with the      //
// source code and can be found at <https://www.gnu.org/licenses/>.           //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#include "Type.h"
#include "Misc/String.h"
#include "Debug.h"

#include "Scheduler/Core.h"
#include "Memory/Core.h"

#define _A(name) _API_##name name

// This structure will always be forward compatible with newer kernels.
// It is a giant table of function pointers with substructures
// to categorize them.
//
// Its just like Java!
// Examples:
// System.Misc.String.Strlen
// System.PnP.Irq.InRequestBusIRQ
// System.PnP.Event.RaiseEvent
//

tstruct {
    struct{
        struct {}SegUtils;
        struct {}BitArray;
        struct {
            _A(StrLen);
            _A(StrCpy);
            _A(StrLower);
            _A(StrUpper);
            _A(Uint32ToString);
        }String;
    }Misc;

    struct {
        _A(Svint86);
        _A(HookDosTrap);
    }Scheduler;

    struct {
        struct {}Irq;
        struct {}Event;
    }PnP;

    struct {
    }Memory;

    struct {
        _A(Logf);
    }Debug;

}KERNEL_API_TABLE;

//
// Some speed-critical code like loops can benefit from reduced call overhead.
// The compiler will probably do this for you. This is just an option.
// The procedure name MUST NOT BE FROM THE STRUCTURE!
//
#define CALLBACK_TYPE(proc_name) _API_##proc_name
