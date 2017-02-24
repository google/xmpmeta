// Copyright 2016 The XMPMeta Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
////////////////////////////////////////////////////////////////////////////////

// This is not your usual header guard. The macro XMPMETA_WARNINGS_DISABLED
// shows up again in reenable_warnings.h.
#ifndef XMPMETA_WARNINGS_DISABLED
#define XMPMETA_WARNINGS_DISABLED

#ifdef _MSC_VER
#pragma warning( push )
// Disable the warning C4251 which is triggered by stl classes in
// xmpmeta's public interface. To quote MSDN: "C4251 can be ignored "
// "if you are deriving from a type in the Standard C++ Library"
#pragma warning( disable : 4251 )
#endif

#endif  // XMPMETA_WARNINGS_DISABLED
