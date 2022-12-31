#include <stdlib.h>
#include <stdio.h>
#include "extmem.h"

void record2XY(unsigned char *blk, int recordNum, int *X, int *Y)
{
    char str[5];
    for (int k = 0; k < 4; k++)
    {
        str[k] = blk[recordNum * 8 + k];
    }
    *X = atoi(str);
    for (int k = 0; k < 4; k++)
    {
        str[k] = blk[recordNum * 8 + 4 + k];
    }
    *Y = atoi(str);
}

void XY2record(unsigned char *blk, int recordNum, int X, int Y)
{
    unsigned char fourBytes[4];
    if (X != -1)
    {
        sprintf(fourBytes, "%d", X);
        for (int k = 0; k < 4; k++)
        {
            blk[recordNum * 8 + k] = fourBytes[k];
        }
    }
    else
    {
        for (int k = 0; k < 4; k++)
        {
            blk[recordNum * 8 + k] = 0;
        }
    }
    if (Y != -1)
    {
        sprintf(fourBytes, "%d", Y);
        for (int k = 0; k < 4; k++)
        {
            blk[recordNum * 8 + 4 + k] = fourBytes[k];
        }
    }
    else
    {
        for (int k = 0; k < 4; k++)
        {
            blk[recordNum * 8 + 4 + k] = 0;
        }
    }
}

int nextAddr(unsigned char *blk)
{
    char str[5];
    for (int k = 0; k < 4; k++)
    {
        str[k] = blk[7 * 8 + k];
    }
    return atoi(str);
}

/**
* ����ֵ��
*   -1�����Ѿ����������м�¼
*   0����ֻ�Ǻ��Ƽ�¼����û�ж����¿�
*   ��0��-1����������¿飬���ص����¿��
*/
int shiftRecord(Buffer *buf, unsigned char **blk, int *recordCnt, int maxRecordCnt)
{
    ++(*recordCnt);
    // ����Ѿ��ﵽ���һ����¼�����������
    if (*recordCnt == maxRecordCnt)
    {
        return -1;
    }
    // �����һ����¼����һ�����У��������һ��
    else if (*recordCnt % 7 == 0)
    {
        int nxtAddr = nextAddr(*blk);
        freeBlockInBuffer(*blk, buf);
        if ((*blk = readBlockFromDisk(nxtAddr, buf)) == NULL)
        {
            perror("Reading Block Failed!\n");
            return -1;
        }
        return nxtAddr;
    }
    return 0;
}

int writeToOutBlk(Buffer *buf, unsigned char **outBlk, int *recordCnt, int *outAddr, int X, int Y)
{
    // ����Ѿ�д��һ�飬�����������
    // Ҫ�ų��ǵ�һ����¼�����
    if (*recordCnt % 7 == 0 && *recordCnt != 0)
    {
        XY2record(*outBlk, 7, *outAddr + 1, -1);
        if (writeBlockToDisk(*outBlk, (*outAddr)++, buf) != 0)
        {
            perror("Writing Block Failed!\n");
            return -1;
        }
        *outBlk = getNewBlockInBuffer(buf);
        printf("ע�����д����̣�%d\n", *outAddr - 1);
    }
    // �洢����������
    XY2record(*outBlk, (*recordCnt)++ % 7, X, Y);
}
