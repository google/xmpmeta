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

#ifndef XMPMETA_PUBLIC_VERSION_H_
#define XMPMETA_PUBLIC_VERSION_H_

#define XMPMETA_VERSION_MAJOR 1
#define XMPMETA_VERSION_MINOR 0
#define XMPMETA_VERSION_REVISION 0

// Classic CPP stringifcation; the extra level of indirection allows the
// preprocessor to expand the macro before being converted to a string.
#define XMPMETA_TO_STRING_HELPER(x) #x
#define XMPMETA_TO_STRING(x) XMPMETA_TO_STRING_HELPER(x)

// The xmpmeta version as a string; for example "1.9.0".
#define XMPMETA_VERSION_STRING XMPMETA_TO_STRING(XMPMETA_VERSION_MAJOR) "." \
                               XMPMETA_TO_STRING(XMPMETA_VERSION_MINOR) "." \
                               XMPMETA_TO_STRING(XMPMETA_VERSION_REVISION)

#endif  // XMPMETA_PUBLIC_VERSION_H_
