#include <stdlib.h>
#include <stdio.h>
#include "extmem.h"

void sortUnion()
{
    printf("------------------------\n");
    printf("��������ļ��ϵĲ��㷨: \n");
    printf("------------------------\n");

    Buffer buf; /* A buffer */
    // ÿ����ϵ����1���ڴ�����ڹ鲢����1���ڴ�黺��鲢��������������
    int groups = 2;
    unsigned char *blk;
    // �洢ÿ����ϵ�����ڴ�Ŀ�ָ��
    unsigned char *blks[groups];
    // ��¼�������д���˵ڼ�����¼����0��ʼд��0��6��
    int recordCnt = 0;
    // ��¼�����Ҫ������Ĵ��̿�
    int outAddr = 801;
    // ��¼��ǰ���ڴ���ÿ����ϵ�ĵڼ�����¼
    // �� 0 ��ʼ����ϵ R �� 112 ������ϵ S �� 224 ��
    int recordCnts[groups];
    int RmaxRecordCnt = 112;
    int SmaxRecordCnt = 224;

    /* Initialize the buffer */
    if (!initBuffer(520, 64, &buf))
    {
        perror("Buffer Initialization Failed!\n");
        return -1;
    }

    // �Ƚ�ÿ����ϵ�ĵ�0������ڴ�
    recordCnts[0] = 0;
    if ((blks[0] = readBlockFromDisk(301, &buf)) == NULL)
    {
        perror("Reading Block Failed!\n");
        return -1;
    }
    recordCnts[1] = 0;
    if ((blks[1] = readBlockFromDisk(317, &buf)) == NULL)
    {
        perror("Reading Block Failed!\n");
        return -1;
    }

    // ����1����Ϊ�������
    blk = getNewBlockInBuffer(&buf);
    // ����ÿ����ϵÿ��ĵ�ǰ��¼����ȵ�ȥ�أ���֤�������
    int A = -1;
    int B = -1;
    int C = -1;
    int D = -1;
    int noMoreR = 0, noMoreS = 0;
    while(!noMoreR || !noMoreS)
    {
        record2XY(blks[0], recordCnts[0] % 7, &A, &B);
        record2XY(blks[1], recordCnts[1] % 7, &C, &D);
        // ��Ҫȥ�أ�ֻ���1��
        if (A == C && B == D && !noMoreR && !noMoreS)
        {
            if (writeToOutBlk(&buf, &blk, &recordCnt, &outAddr, A, B) == -1)
            {
                return -1;
            }
            if (shiftRecord(&buf, &blks[0], &recordCnts[0], RmaxRecordCnt) == -1)
            {
                noMoreR = 1;
            }
            if (shiftRecord(&buf, &blks[1], &recordCnts[1], SmaxRecordCnt) == -1)
            {
                noMoreS = 1;
            }
        }
        // ��ϵR�����
        else if ((noMoreS || A < C || (A == C && B < D)) && !noMoreR)
        {
            if (writeToOutBlk(&buf, &blk, &recordCnt, &outAddr, A, B) == -1)
            {
                return -1;
            }
            if (shiftRecord(&buf, &blks[0], &recordCnts[0], RmaxRecordCnt) == -1)
            {
                noMoreR = 1;
            }
        }
        // ��ϵS�����
        else if (!noMoreS)
        {
            if (writeToOutBlk(&buf, &blk, &recordCnt, &outAddr, C, D) == -1)
            {
                return -1;
            }
            if (shiftRecord(&buf, &blks[1], &recordCnts[1], SmaxRecordCnt) == -1)
            {
                noMoreS = 1;
            }
        }
    }
    // �������ʣ�����Ӽ�¼����������һ��
    if (recordCnt)
    {
        if (writeBlockToDisk(blk, outAddr, &buf) != 0)
        {
            perror("Writing Block Failed!\n");
            return -1;
        }
        printf("ע�����д����̣�%d\n", outAddr);
    }
    printf("\n");
    printf("S��R�Ĳ�����%d��Ԫ�ء�\n", recordCnt);
    printf("\n");
    freeBuffer(&buf);
    return 0;
}
