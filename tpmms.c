#include <stdlib.h>
#include <stdio.h>
#include "extmem.h"

#define SUBSET_BLK_NUM 8 // 8��Ϊһ���Ӽ�/һ��
#define INTERNAL_R_BLK 601
#define INTERNAL_S_BLK 617

void internalSort(Buffer *buf, int beginBlk, int endBlk)
{
    unsigned char *blks[SUBSET_BLK_NUM]; // �洢ÿ���ڴ���ָ��
    for (int i = beginBlk, nxtAddr = beginBlk; nxtAddr && i <= endBlk; i++)
    {
        if ((blks[(i - 1) % SUBSET_BLK_NUM] = readBlockFromDisk(nxtAddr, buf)) == NULL)
        {
            perror("Reading Block Failed!\n");
            return -1;
        }
        nxtAddr = nextAddr(blks[(i - 1) % SUBSET_BLK_NUM]);
        // printf("�������ݿ�%d\n", i);
        // ÿ����SUBSET_BLK_NUM���飬����һ��������
        // ����һ��������������ܿ������������������һ��δ�������
        if (i % SUBSET_BLK_NUM == 0)
        {
            printf("��ʼ�������%d�鵽��%d��\n", i - 7, i);
            // ð������
            // X��Y���ڴ洢���Ƚϵ�������¼
            int X1 = -1;
            int Y1 = -1;
            int X2 = -1;
            int Y2 = -1;
            for (int j = 0; j < SUBSET_BLK_NUM * 7 - 1; j++)
            {
                for (int k = 0; k < SUBSET_BLK_NUM * 7 - (j + 1); k++)
                {
                    record2XY(blks[k / 7], k % 7, &X1, &Y1);
                    record2XY(blks[(k + 1) / 7], (k + 1) % 7, &X2, &Y2);
                    if (X1 > X2 || (X1 == X2 && Y1 > Y2))
                    {
                        XY2record(blks[k / 7], k % 7, X2, Y2);
                        XY2record(blks[(k + 1) / 7], (k + 1) % 7, X1, Y1);
                    }

                }
            }

            for (int j = 0; j < SUBSET_BLK_NUM; j++)
            {
                // ����������ϵ�����һ�飬�����鶼Ҫ���ú�̿��ַ
                if (i == endBlk && j == SUBSET_BLK_NUM - 1)
                {
                    XY2record(blks[j], 7, -1, -1);
                }
                else
                {
                    XY2record(blks[j], 7, INTERNAL_R_BLK + i - SUBSET_BLK_NUM + j + 1, -1);
                }
                if (writeBlockToDisk(blks[j],
                                 INTERNAL_R_BLK + i - SUBSET_BLK_NUM + j,
                                 buf) != 0)
                {
                    perror("Writing Block Failed!\n");
                    return -1;
                }

            }
            printf("��������������������%d������%d��\n",
                   INTERNAL_R_BLK + i - SUBSET_BLK_NUM,
                   INTERNAL_R_BLK + i - 1);
        }
    }
}

void externalSort(Buffer *buf, int beginBlk, int endBlk)
{
    // ÿ�����1���ڴ�����ڹ鲢����1���ڴ�黺��鲢��������������
    // ��������������Ҳ�����7�飬�ڴ�鹻��
    int groups = (endBlk - beginBlk + 1) / SUBSET_BLK_NUM;
    unsigned char *blk;
    // �洢ÿ������ڴ�Ŀ�ָ��
    unsigned char *blks[groups];
    // ��¼�������д���˵ڼ�����¼����0��ʼд��0��6��
    int recordCnt = 0;
    // ��¼��ǰ���ڴ���ÿ��ĵڼ�����¼����0��ʼ��ÿ�鹲 SUBSET_BLK_NUM * 7 ����
    int recordCnts[groups];

    // �Ƚ�ÿ��ĵ�0������ڴ�
    for (int i = 0; i < groups; i++)
    {
        recordCnts[i] = 0;
        if ((blks[i] = readBlockFromDisk(beginBlk + i * SUBSET_BLK_NUM, buf)) == NULL)
        {
            perror("Reading Block Failed!\n");
            return -1;
        }
    }
    // ����1����Ϊ�������
    blk = getNewBlockInBuffer(buf);
    // ����ÿ��ÿ��ĵ�ǰ��¼���ҵ���С��¼
    int Xpre;
    int Ypre;
    int Xcur;
    int Ycur;
    // ��¼ÿ�α���ѡ��鲢��¼����
    int select;
    // ÿ�ι鲢һ����¼��������棬�� groups * SUBSET_BLK_NUM * 7 ��
    for (int k = 0; k < groups * SUBSET_BLK_NUM * 7; k++)
    {
        Xpre = -1;
        Ypre = -1;
        Xcur = -1;
        Ycur = -1;
        select = -1;
        for (int i = 0; i < groups; i++)
        {
            // ��������Ѿ������������һ����¼��������
            if (recordCnts[i] == -1)
            {
                continue;
            }
            record2XY(blks[i], recordCnts[i] % 7, &Xcur, &Ycur);
            if (Xcur < Xpre || (Xcur == Xpre && Ycur < Ypre) || Xpre == -1)
            {
                Xpre = Xcur;
                Ypre = Ycur;
                select = i;
            }
        }
        // ����С��¼д���������飬�����Ƹ���ĵ�ǰ�����¼
        XY2record(blk, recordCnt++, Xpre, Ypre);
        recordCnts[select]++;
        // �����������д���˵�7���������������
        if (recordCnt == 7)
        {
            // ����������һ������鲢�����Ǻ�̵�ַ
            if (k != groups * SUBSET_BLK_NUM * 7 - 1)
            {
                XY2record(blk, 7, 301 + beginBlk - INTERNAL_R_BLK + k / 7 + 1, -1);
            }
            if (writeBlockToDisk(blk, 301 + beginBlk - INTERNAL_R_BLK + k / 7, buf) != 0)
            {
                perror("Writing Block Failed!\n");
                return -1;
            }
            blk = getNewBlockInBuffer(buf);
            recordCnt = 0;
            printf("����鲢�������%d��\n", 301 + beginBlk - INTERNAL_R_BLK + k / 7);
        }
        // ������α�ѡ�е����Ѿ����������һ����¼��������Ϊ-1
        if (recordCnts[select] == SUBSET_BLK_NUM * 7)
        {
            printf("�������%d��Ĵ��̿�%d�����鴦�����\n",
                   select,
                   beginBlk + select * SUBSET_BLK_NUM + recordCnts[select] / 7 - 1);
            recordCnts[select] = -1;
        }
        // ����������һ����¼�����Ѿ�������һ���飬�������һ��
        else if (recordCnts[select] % 7 == 0)
        {
            int nxtAddr = nextAddr(blks[select]);
            freeBlockInBuffer(blks[select], buf);
            if ((blks[select] = readBlockFromDisk(nxtAddr, buf)) == NULL)
            {
                perror("Reading Block Failed!\n");
                return -1;
            }
            printf("�������%d��Ĵ��̿�%d��������һ��\n",
                   select,
                   beginBlk + select * SUBSET_BLK_NUM + recordCnts[select] / 7 - 1);
        }
    }
}

void tpmms()
{
    printf("------------------------\n");
    printf("���׶ζ�·�鲢�����㷨: \n");
    printf("------------------------\n");

    Buffer buf; /* A buffer */

    int i = 0;


    /* Initialize the buffer */
    if (!initBuffer(520, 64, &buf))
    {
        perror("Buffer Initialization Failed!\n");
        return -1;
    }

    // �ֱ��������ϵR�͹�ϵS
    // �������м����ݴ���501-548��
    internalSort(&buf, 1, 16);
    internalSort(&buf, 17, 48);

    // �ֱ��������ϵR�͹�ϵS
    // �����������洢��301-348��
    externalSort(&buf, INTERNAL_R_BLK, INTERNAL_R_BLK + 15);
    externalSort(&buf, INTERNAL_S_BLK, INTERNAL_S_BLK + 31);

    // �����������м���
    for (int i = INTERNAL_R_BLK; i <= INTERNAL_S_BLK + 31; i++)
    {
        dropBlockOnDisk(i);
    }
    printf("�Ѳ������̿�%d�����̿�%d�ϵ��м���\n", INTERNAL_R_BLK, INTERNAL_S_BLK + 31);
    printf("\n");

    freeBuffer(&buf);

}
