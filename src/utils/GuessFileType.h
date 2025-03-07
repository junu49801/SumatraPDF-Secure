
/* Copyright 2022 the SumatraPDF project authors (see AUTHORS file).
   License: Simplified BSD (see COPYING.BSD) */

extern Kind kindFilePDF;
extern Kind kindFilePS;
extern Kind kindFileXps;
extern Kind kindFileDjVu;
extern Kind kindFileChm;

extern Kind kindFileZip;
extern Kind kindFileCbz;
extern Kind kindFileCbr;
extern Kind kindFileRar;
extern Kind kindFile7Z;
extern Kind kindFileCb7;
extern Kind kindFileTar;
extern Kind kindFileCbt;

extern Kind kindFilePng;
extern Kind kindFileJpeg;
extern Kind kindFileGif;
extern Kind kindFileTiff;
extern Kind kindFileBmp;
extern Kind kindFileTga;
extern Kind kindFileJxr;
extern Kind kindFileHdp;
extern Kind kindFileWdp;
extern Kind kindFileWebp;
extern Kind kindFileJp2;

extern Kind kindFileFb2;
extern Kind kindFileFb2z;
extern Kind kindFileEpub;
extern Kind kindFileMobi;
extern Kind kindFilePalmDoc;
extern Kind kindFileHTML;
extern Kind kindFileSvg;
extern Kind kindFileHeic;
extern Kind kindFileAvif;
extern Kind kindFileTxt;

extern Kind kindDirectory;

const char* FindEmbeddedPdfFileStreamNo(const char* path);

Kind GuessFileTypeFromContent(const char* path);
Kind GuessFileTypeFromContent(const ByteSlice& d);
Kind GuessFileTypeFromName(const char*);
Kind GuessFileType(const char* path, bool sniff);
const char* GfxFileExtFromData(const ByteSlice&);
const char* GfxFileExtFromKind(Kind);

bool KindInArray(Kind* kinds, int nKinds, Kind kind);
