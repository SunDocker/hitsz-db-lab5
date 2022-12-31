#include <stdlib.h>
#include <stdio.h>
#include "extmem.h"

int buildIndex(int beginIdx)
{
    Buffer buf; /* A buffer */
    unsigned char *dataBlk; // ���ݿ�
    unsigned char *idxBlk; // ������
    int idxCnt = 0; // ��¼��ǰ���ڴ���������ĵڼ���ָ��
    int idxOutAddr = 350; // ��¼�������Ҫ������Ĵ��̿��
    int idxData = -1, Y = -1;
    int preIdxData = -1;

    /* Initialize the buffer */
    // 1���ڴ�����������飬Ϊ7�����̿齨������
    // 1���ڴ�������ݴ�����
    if (!initBuffer(520, 64, &buf))
    {
        perror("Buffer Initialization Failed!\n");
        return -1;
    }
    idxBlk = getNewBlockInBuffer(&buf);
    for (int i = beginIdx; i;)
    {
        if ((dataBlk = readBlockFromDisk(i, &buf)) == NULL)
        {
            perror("Reading Block Failed!\n");
            return -1;
        }
        // ��ȡ���ݿ��һ�����ݣ����ڴ���ָ��
        record2XY(dataBlk, 0, &idxData, &Y);
        // ���������д���һ��ָ�루������ֵ���ظ���
        if (idxData != preIdxData)
        {
            preIdxData = idxData;
            XY2record(idxBlk, idxCnt, idxData, i);
            // ������������Ѿ�����7��ָ�룬�����������
            if (++idxCnt == 7)
            {
                XY2record(idxBlk, idxCnt, idxOutAddr + 1, -1);
                if (writeBlockToDisk(idxBlk, idxOutAddr++, &buf) != 0)
                {
                    perror("Writing Block Failed!\n");
                    return -1;
                }
                idxBlk = getNewBlockInBuffer(&buf);
                idxCnt = 0;
            }
        }
        i = nextAddr(dataBlk);
        freeBlockInBuffer(dataBlk, &buf);
    }
    // ������һ������7��ָ���������
    if (idxCnt != 0)
    {
        if (writeBlockToDisk(idxBlk, idxOutAddr, &buf) != 0)
        {
            perror("Writing Block Failed!\n");
            return -1;
        }
    }

    freeBuffer(&buf);

    return 350;
}

void indexSelect()
{
    int C = 128;
    printf("------------------------------\n");
    printf("����������ѡ���㷨 S.C = %d: \n", C);
    printf("------------------------------\n");

    // ΪS.C��������
    int idxBeginAddr = buildIndex(317);

    Buffer buf; /* A buffer */
    unsigned char *dataBlk; // ���ݿ�
    unsigned char *idxBlk; // ������
    unsigned char *resBlk; // ����������������ġ��������ѡ����
    int rowCount = 0; // ���������ļ�¼��
    int idxData = -1, idxPtr = -1;

    /* Initialize the buffer */
    if (!initBuffer(520, 64, &buf))
    {
        perror("Buffer Initialization Failed!\n");
        return -1;
    }

    for (int i = idxBeginAddr; i;)
    {
        if ((idxBlk = readBlockFromDisk(i, &buf)) == NULL)
        {
            perror("Reading Block Failed!\n");
            return -1;
        }
        printf("����������%d\n", i);
        int flag = 0;
        int j = 0;
        for (j = 0; j < 7; j++)
        {
            record2XY(idxBlk, j, &idxData, &idxPtr);
            // ��һ���ҵ�����ֵ����Ŀ��ֵ������������������
            if (idxData > C)
            {
                flag = 1;
                break;
            }
        }
        // ��һ���ҵ�����ֵ����Ŀ��ֵ������������������
        if (flag)
        {
            int preIdxData = -1, preIdxPtr = -1;
            int X = -1, Y = -1;

            if (j)
            {
                record2XY(idxBlk, j - 1, &preIdxData, &preIdxPtr);

            }
            // �����һ��������ڵ�ǰ�������У�����Ҫ�ٴζ�����һ������
            else
            {
                if (i)
                {
                    if ((idxBlk = readBlockFromDisk(i - 1, &buf)) == NULL)
                    {
                        perror("Reading Block Failed!\n");
                        return -1;
                    }
                    printf("����������%d\n", i - 1);
                    record2XY(idxBlk, 6, &preIdxData, &preIdxPtr);
                }
                // �����ǰ�ľ��ǵ�1�������飬˵��������ϵ��û�����������ļ�¼
                else
                {
                    break;
                }
            }
            int noMore = 0;
            for (int k = preIdxPtr; k && k < idxPtr;)
            {
                if ((dataBlk = readBlockFromDisk(k, &buf)) == NULL)
                {
                    perror("Reading Block Failed!\n");
                    return -1;
                }
                printf("�������ݿ�%d\n", k);
                for (int l = 0; l < 7; l++)
                {
                    record2XY(dataBlk, l, &X, &Y);
                    if (X == C)
                    {
                        printf("(X=%d, Y=%d)\n", X, Y);
                        // ÿ����7���������һ�ν��
                        if ((++rowCount - 1) % 7 == 0)
                        {
                            // ��Ҫ�ų�һ�¸յõ�һ����������
                            if (rowCount != 1) {
                                // ����һ���ַд���ڴ��
                                XY2record(resBlk, 7, 120 + (rowCount / 7), -1);
                                // ���ڴ��д����̿�
                                if (writeBlockToDisk(resBlk, 120 + (rowCount / 7) - 1, &buf) != 0)
                                {
                                    perror("Writing Block Failed!\n");
                                    return -1;
                                }
                            }
                            resBlk = getNewBlockInBuffer(&buf);
                        }
                        XY2record(resBlk, (rowCount - 1) % 7, X, Y);
                    }
                    // ��¼������ģ�
                    // �����ǰ��¼�Ѿ�����Ŀ��ֵ������治�����������������ļ�¼
                    else if (X > C)
                    {
                        noMore = 1;
                        break;
                    }
                }
                k = nextAddr(dataBlk);
                freeBlockInBuffer(dataBlk, &buf);
                if (noMore)
                {
                    break;
                }
            }
            break;
        }
        else
        {
            printf("û������������Ԫ��\n");
        }
    }

    // �����һ��������7����¼�Ŀ�д�����
    if (resBlk && writeBlockToDisk(resBlk, 120 + (rowCount - 1) / 7, &buf) != 0)
    {
        perror("Writing Block Failed!\n");
        return -1;
    }

    // ���д����̵���Ϣ
    for (int i = 1; i <= rowCount; i += 7)
    {
        printf("ע�����д����̣�%d\n", 120 + (i - 1) / 7);
    }
    printf("\n");
    printf("����ѡ��������Ԫ��һ��%d��\n", rowCount);
    printf("\n");
    printf("IO��дһ��%d��\n", buf.numIO); /* Check the number of IO's */
    printf("\n");

    freeBuffer(&buf);

    return 0;
}
