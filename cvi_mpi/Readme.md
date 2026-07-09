# cvi_mpi repository overview

Copyright (c) 2023, Sophgo Technologies. All rights reserved.

## Purpose
This repository provides the CVI MPI (Media Processing Interface) and related components, samples, and tools used to build, test, and integrate media processing functionality on Sophgo platforms.

## Release directory structure

```sh
.
├── 3rdparty			# External third-party components used by the project.
│   ├── inih			# INI parser with its own LICENSE.txt (BSD 3-Clause).
│   ├── Makefile
│   └── openssl			# Static libraries (*.a) and headers.
├── component			# Panel components header files.
├── dual_os.mk
├── include				# Public header files for ISP APIs and shared interfaces.
├── lib
├── linux.mk
├── Makefile
├── Makefile.param
├── modules				# Optional modules and platform-specific code.
│   ├── cvi_bin
│   ├── sys
│   │   ├── api
│   │   ├── include
│   │   ├── Makefile
│   │   └── platform
│   └── ...
├── pkgconfig			# .pc files and metadata to help consumers link against provided libraries.
├── sample_app			# Example applications demonstrating API usage and integration patterns.
├── self_test			# Self-test utilities
└── tool				# tooling scripts.
```

## Licensing overview
### Company code (Sophgo original code)
Unless a file explicitly states another license in its header, Sophgo’s original source code in this repository is licensed under the BSD 3-Clause License. See LICENSE at repository root.

Exception: 

  - lib/ directory contents (*.a, *.so) are proprietary and not open source; redistribution and usage are governed by Sophgo’s separate agreement.

### Third-party components:

  - 3rdparty/inih: The 3-Clause BSD License (see 3rdparty/inih/LICENSE.txt).

  - 3rdparty/openssl: Apache License 2.0 (see https://www.openssl.org/source/license.html; changes noted in 3rdparty/openssl/openssl.patch).

  - modules/cvi_bin: Mixed. Some *.py files in this module declare "License: BSD". modules/cvi_bin/python contains LICNES files declared The MIT/X Consortium license.

  - modules/sys/platform: Mixed
   - Files marked "Apache License, Version 2.0": Apache-2.0.
   - Files marked "SPDX-License-Identifier: GPL-2.0": GPL-2.0-only or GPL-2.0-or-later, as per file headers.

SPDX usage and per-file authority
Each source file’s header (and its SPDX-License-Identifier where present) is authoritative for that file’s license. The top-level license policy applies to company code that does not include a specific per-file license header.

### Non-open-source notice

The lib/ directory contains proprietary libraries. 

They are not covered by the open-source licenses stated elsewhere in this repository. Redistribution and use require a separate agreement with Sophgo.

## Compliance notes

If you distribute binaries that incorporate GPL/LGPL components, ensure full compliance with corresponding source and linking obligations.

OpenSSL usage is governed by its license terms;
verify your distribution meets OpenSSL’s notice/acknowledgment requirements.

The repository may include mixed-licensed modules; 
consult file headers when combining or redistributing code.

## Contact

For licensing questions, redistribution rights, or commercial support,  welcome to submit issues.

## Additional references
BSD 3-Clause: https://opensource.org/license/bsd-3-clause/
Apache-2.0: https://www.apache.org/licenses/LICENSE-2.0
GPL-2.0: https://www.gnu.org/licenses/old-licenses/gpl-2.0.html
OpenSSL License: https://www.openssl.org/source/license.html
