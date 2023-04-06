//
//  FileUtility.h
//  HappyLib
//
//  Created by Nyon on 12/12/17.
//  Copyright © 2017 Happy Labs Pte Ltd. All rights reserved.
//

#ifndef FileUtility_h
#define FileUtility_h

#include <string>

namespace FileUtility {
    BOOLEAN GetFileContent(const CHAR * szFilename, BYTE ** pbyBuffer, UINT * pSize);
    BOOLEAN WriteStringToFile(const CHAR * szFilename, const std::string& data);
};

#endif /* FileUtility_h */
