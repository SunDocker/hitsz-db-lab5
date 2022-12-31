#include <stdlib.h>
#include <stdio.h>
#include "extmem.h"



void sortMergeJoin()
{
    printf("--------------------\n");
    printf("��������������㷨: \n");
    printf("--------------------\n");

    Buffer buf; /* A buffer */
    // ÿ����ϵ����1���ڴ�����ڹ鲢����1���ڴ�黺��鲢��������������
    int groups = 2;
    unsigned char *blk;
    // �洢ÿ����ϵ�����ڴ�Ŀ�ָ��
    unsigned char *blks[groups];
    // ��¼�������д���˵ڼ�����¼����0��ʼд��0��5������������¼��һ�����ӽ����
    int recordCnt = 0;
    // ��¼���ӽ��Ҫ������Ĵ��̿�
    int outAddr = 401;
    // ��¼��ǰ���ڴ���ÿ����ϵ�ĵڼ�����¼
    // �� 0 ��ʼ����ϵ R �� 112 ������ϵ S �� 224 ��
    int recordCnts[groups];
    int RmaxRecordCnt = 112;
    int SmaxRecordCnt = 224;
    int curRBlk;

    /* Initialize the buffer */
    if (!initBuffer(520, 64, &buf))
    {
        perror("Buffer Initialization Failed!\n");
        return -1;
    }

    // �Ƚ�ÿ����ϵ�ĵ�0������ڴ�
    recordCnts[0] = 0;
    curRBlk = 0;
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
    // ����ÿ����ϵÿ��ĵ�ǰ��¼���ҵ��������ӵļ�¼
    int A = -1;
    int B = -1;
    int C = -1;
    int D = -1;
    while(recordCnts[0] < RmaxRecordCnt && recordCnts[1] < SmaxRecordCnt)
    {
        record2XY(blks[0], recordCnts[0] % 7, &A, &B);
        record2XY(blks[1], recordCnts[1] % 7, &C, &D);
        // printf("��ȡ�˹�ϵR�ĵ�%d������ϵS�ĵ�%d��\n", recordCnts[0], recordCnts[1]);
        // printf("R.A=%d��S.C=%d\n", A, C);
        // ��������
        if (A == C)
        {
            int eqlVal = A;
            // �̶���ϵR��ָ��
            int fixR = recordCnts[0];
            int fixBlk = curRBlk;
            // ������ϵS������ȵļ�¼���ֱ����ӹ�ϵR��������ȵļ�¼
            int RnoMore = 0;
            int noMore = 0;
            while (C == eqlVal)
            {
                // ���ӹ�ϵR��������ȵļ�¼
                recordCnts[0] = fixR;
                if (curRBlk != fixBlk)
                {
                    freeBlockInBuffer(blks[0], &buf);
                    if ((*blks[0] = readBlockFromDisk(fixBlk, &buf)) == NULL)
                    {
                        return -1;
                    }
                    curRBlk = fixBlk;
                }
                record2XY(blks[0], recordCnts[0] % 7, &A, &B);
                // printf("��ȡ�˹�ϵR�ĵ�%d��\n", recordCnts[0]);
                // printf("R.A=%d\n", A);
                while (A == eqlVal)
                {
                    // ����Ѿ�д��һ�飬�����������
                    // Ҫ�ų��ǵ�һ����¼�����
                    if (recordCnt % 6 == 0 && recordCnt != 0)
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
                    XY2record(blk, recordCnt++ % 6, A, B);
                    XY2record(blk, recordCnt++ % 6, C, D);
                    // printf("�����˹�ϵR�ĵ�%d������ϵS�ĵ�%d��\n", recordCnts[0], recordCnts[1]);
                    // ��ϵR����һ����¼
                    int res;
                    if ((res = shiftRecord(&buf, &blks[0], &recordCnts[0], RmaxRecordCnt)) == -1)
                    {
                        RnoMore = 1;
                        break;
                    }
                    if (res)
                    {
                        curRBlk = res;
                    }
                    record2XY(blks[0], recordCnts[0] % 7, &A, &B);
                    // printf("��ȡ�˹�ϵR�ĵ�%d��\n", recordCnts[0]);
                    // printf("R.A=%d\n", A);
                }
                // ��������ӹ�����R�Ѿ��ﵽ���һ����¼�������������ӽ������ٽ�������
                if (RnoMore)
                {
                    noMore = 1;
                }
                // ��ϵS����һ����¼
                if (shiftRecord(&buf, &blks[1], &recordCnts[1], SmaxRecordCnt) == -1)
                {
                    noMore = 1;
                    break;
                }
                record2XY(blks[1], recordCnts[1] % 7, &C, &D);
                // printf("��ȡ�˹�ϵS�ĵ�%d��\n", recordCnts[1]);
                // printf("S.C=%d\n", C);
            }
            // ��������ӹ�����S�Ѿ��ﵽ���һ����¼�����������
            if (noMore)
            {
                break;
            }
        }
        // ��ϵR����һ����¼
        else if (A < C)
        {
            int res;
            if ((res = shiftRecord(&buf, &blks[0], &recordCnts[0], RmaxRecordCnt)) == -1)
            {
                break;
            }
            // ��Ҫ��¼��ϵR��������һ��
            if (res)
            {
                curRBlk = res;
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
    printf("�ܹ�����%d�Ρ�\n", recordCnt / 2);
    printf("\n");
    freeBuffer(&buf);
}
