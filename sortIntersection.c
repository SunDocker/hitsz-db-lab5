#include <stdlib.h>
#include <stdio.h>
#include "extmem.h"

int sortIntersection()
{
    printf("------------------------\n");
    printf("��������ļ��ϵĽ��㷨: \n");
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
    int outAddr = 140;
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
    // ����ÿ����ϵÿ��ĵ�ǰ��¼���ҵ���ȵļ�¼
    int A = -1;
    int B = -1;
    int C = -1;
    int D = -1;
    while(recordCnts[0] < RmaxRecordCnt && recordCnts[1] < SmaxRecordCnt)
    {
        record2XY(blks[0], recordCnts[0] % 7, &A, &B);
        record2XY(blks[1], recordCnts[1] % 7, &C, &D);
        // ���ɹ�
        if (A == C && B == D)
        {
            // ����Ѿ�д��һ�飬�����������
            // Ҫ�ų��ǵ�һ����¼�����
            /* if (recordCnt % 7 == 0 && recordCnt != 0)
            {
                XY2record(blk, 7, outAddr + 1, -1);
                if (writeBlockToDisk(blk, outAddr++, &buf) != 0)
                {
                    perror("Writing Block Failed!\n");
                    return -1;
                }
                blk = getNewBlockInBuffer(&buf);
                printf("ע�����д����̣�%d\n", outAddr - 1);
            }
            // �洢����������
            XY2record(blk, recordCnt++ % 7, A, B); */
            if (writeToOutBlk(&buf, &blk, &recordCnt, &outAddr, A, B) == -1)
            {
                return -1;
            }
            printf("(X=%d, Y=%d)\n", A, B);
            if (shiftRecord(&buf, &blks[0], &recordCnts[0], RmaxRecordCnt) == -1)
            {
                break;
            }
            if (shiftRecord(&buf, &blks[1], &recordCnts[1], SmaxRecordCnt) == -1)
            {
                break;
            }
        }
        // ��ϵR����һ����¼
        else if (A < C || (A == C && B < D))
        {
            if (shiftRecord(&buf, &blks[0], &recordCnts[0], RmaxRecordCnt) == -1)
            {
                break;
            }
        }
        // ��ϵS����һ����¼
        else
        {
            if (shiftRecord(&buf, &blks[1], &recordCnts[1], SmaxRecordCnt) == -1)
            {
                break;
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
    printf("S��R�Ľ�����%d��Ԫ�ء�\n", recordCnt);
    printf("\n");
    freeBuffer(&buf);

    return 0;

}
