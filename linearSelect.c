#include <stdlib.h>
#include <stdio.h>
#include "extmem.h"

void linearSelect()
{
    printf("----------------------------------\n");
    printf("��������������ѡ���㷨 S.C = 128: \n");
    printf("----------------------------------\n");
    Buffer buf; /* A buffer */
    unsigned char *blk; // �������϶�ȡ������
    int rowCount = 0; // ���������ļ�¼��
    unsigned char *resBlk; // ����������������ġ��������ѡ����

    /* Initialize the buffer */
    // 1���ڴ���������϶�ȡ������
    // 1���ڴ��������ſ��ܵ�ѡ����
    if (!initBuffer(520, 64, &buf))
    {
        perror("Buffer Initialization Failed!\n");
        return -1;
    }

    /* Read the block from the hard disk */
    for (int i = 17; i && i <= 48;)
    {
        if ((blk = readBlockFromDisk(i, &buf)) == NULL)
        {
            perror("Reading Block Failed!\n");
            return -1;
        }

        /* Process the data in the block */
        int X = -1;
        int Y = -1;
        int addr = -1;
        for (int j = 0; j < 7; j++) //һ��blk��7��Ԫ���һ����ַ
        {
            record2XY(blk, j, &X, &Y);
            if (X == 128)
            {
                printf("(X=%d, Y=%d)\n", X, Y);
                // ÿ����7���������һ�ν��
                if ((++rowCount - 1) % 7 == 0)
                {
                    // ��Ҫ�ų�һ�¸յõ�һ����������
                    if (rowCount != 1) {
                        // ����һ���ַд���ڴ��
                        XY2record(resBlk, 7, 100 + (rowCount / 7), -1);
                        // ���ڴ��д����̿�
                        if (writeBlockToDisk(resBlk, 100 + (rowCount / 7) - 1, &buf) != 0)
                        {
                            perror("Writing Block Failed!\n");
                            return -1;
                        }
                    }
                    resBlk = getNewBlockInBuffer(&buf);
                }
                XY2record(resBlk, (rowCount - 1) % 7, X, Y);
            }
        }
        i = nextAddr(blk);
        freeBlockInBuffer(blk, &buf);
    }


    // �����һ��������7����¼�Ŀ�д�����
    if (resBlk && writeBlockToDisk(resBlk, 100 + (rowCount - 1) / 7, &buf) != 0)
    {
        perror("Writing Block Failed!\n");
        return -1;
    }

    // ���д����̵���Ϣ
    for (int i = 1; i <= rowCount; i += 7)
    {
        printf("ע�����д����̣�%d\n", 100 + (i - 1) / 7);
    }
    printf("\n");
    printf("����ѡ��������Ԫ��һ��%d��\n", rowCount);
    printf("\n");
    printf("IO��дһ��%d��\n", buf.numIO); /* Check the number of IO's */
    printf("\n");

    freeBuffer(&buf);

    return 0;
}



