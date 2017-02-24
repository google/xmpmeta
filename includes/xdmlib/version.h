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

#ifndef XDMLIB_PUBLIC_VERSION_H_
#define XDMLIB_PUBLIC_VERSION_H_

#define XDMLIB_VERSION_MAJOR 1
#define XDMLIB_VERSION_MINOR 0
#define XDMLIB_VERSION_REVISION 0

// Classic CPP stringifcation; the extra level of indirection allows the
// preprocessor to expand the macro before being converted to a string.
#define XDMLIB_TO_STRING_HELPER(x) #x
#define XDMLIB_TO_STRING(x) XDMLIB_TO_STRING_HELPER(x)

// The xdmlib version as a string; for example "1.9.0".
#define XDMLIB_VERSION_STRING XDMLIB_TO_STRING(XDMLIB_VERSION_MAJOR) "." \
                               XDMLIB_TO_STRING(XDMLIB_VERSION_MINOR) "." \
                               XDMLIB_TO_STRING(XDMLIB_VERSION_REVISION)

#endif  // XDMLIB_PUBLIC_VERSION_H_
