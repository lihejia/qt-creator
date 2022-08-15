#include <utils/porting.h>
static QList<RowData> readLines(StringView patch, bool lastChunk, bool *lastChunkAtTheEndOfFile, bool *ok)
    const QVector<StringView> lines = patch.split(newLine);
        StringView line = lines.at(i);
static StringView readLine(StringView text, StringView *remainingText, bool *hasNewLine)
            *remainingText = StringView();
static bool detectChunkData(StringView chunkDiff, ChunkData *chunkData, StringView *remainingPatch)
    const StringView chunkLine = readLine(chunkDiff, remainingPatch, &hasNewLine);
    StringView leftPos = chunkLine.mid(leftPosStart, leftPosLength);
    StringView rightPos = chunkLine.mid(rightPosStart, rightPosLength);
    const StringView optionalHint = chunkLine.mid(optionalHintStart, optionalHintLength);
static QList<ChunkData> readChunks(StringView patch, bool *lastChunkAtTheEndOfFile, bool *ok)
        const StringView chunkDiff = patch.mid(chunkStart, chunkEnd - chunkStart);
        StringView lines;
static FileData readDiffHeaderAndChunks(StringView headerAndChunks, bool *ok)
    StringView patch = headerAndChunks;
static QList<FileData> readDiffPatch(StringView patch, bool *ok, QFutureInterfaceBase *jobController)
                StringView headerAndChunks = patch.mid(lastPos, pos - lastPos);
            StringView headerAndChunks = patch.mid(lastPos, patch.size() - lastPos - 1);
static bool detectIndexAndBinary(StringView patch, FileData *fileData, StringView *remainingPatch)
    StringView afterNextLine;
    const StringView nextLine = readLine(patch, &afterNextLine, &hasNewLine);
        const StringView indices = nextLine.mid(indexHeader.size());
        *remainingPatch = StringView();
    StringView afterMinuses;
    const StringView minuses = readLine(*remainingPatch, &afterMinuses, &hasNewLine);
    StringView afterPluses;
    const StringView pluses = readLine(afterMinuses, &afterPluses, &hasNewLine);
static bool extractCommonFileName(StringView fileNames, StringView *fileName)
    const StringView leftFileName = fileNames.mid(2, fileNameSize - 2);
    const StringView rightFileName = fileNames.mid(fileNameSize + 3, fileNameSize - 2);
static bool detectFileData(StringView patch, FileData *fileData, StringView *remainingPatch)
    StringView afterDiffGit;
    const StringView diffGit = readLine(patch, &afterDiffGit, &hasNewLine);
    const StringView fileNames = diffGit.mid(gitHeader.size());
    StringView commonFileName;
        StringView afterSecondLine;
        const StringView secondLine = readLine(afterDiffGit, &afterSecondLine, &hasNewLine);
            StringView afterThirdLine;
        StringView afterModeOrSimilarity;
        StringView afterSimilarity;
        const StringView secondLine = readLine(afterDiffGit, &afterModeOrSimilarity, &hasNewLine);
        StringView afterCopyRenameFrom;
        const StringView copyRenameFrom = readLine(afterSimilarity, &afterCopyRenameFrom, &hasNewLine);
        StringView afterCopyRenameTo;
        const StringView copyRenameTo = readLine(afterCopyRenameFrom, &afterCopyRenameTo, &hasNewLine);
static QList<FileData> readGitPatch(StringView patch, bool *ok, QFutureInterfaceBase *jobController)
        StringView patch;
        const StringView fileDiff = patch.mid(diffStart, diffEnd - diffStart);
        StringView remainingFileDiff;
    StringView croppedPatch = make_stringview(patch);