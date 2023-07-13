#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

typedef struct Code
{
    int64_t dec;
    int8_t size;
} Code;

typedef struct Item
{
    unsigned char ch;
    unsigned long freq;
    Code code;
    struct Item *left;
    struct Item *right;
} Item;

int initItem(Item *pItem, char ch)
{
    if (pItem == NULL)
        return 1;
    pItem->ch = ch;
    pItem->freq = 0;
    pItem->left = pItem->right = NULL;
    return 0;
}

void deleteItem(Item *pItem)
{
    if (pItem == NULL)
        return;
    deleteItem(pItem->left);
    deleteItem(pItem->right);
    free(pItem);
}

typedef struct Node
{
    Item *pItem;
    struct Node *left;
    struct Node *right;
} Node;

int initNode(Node *pNode, Item item)
{
    if (pNode == NULL)
        return 1;
    pNode->pItem = (Item*)malloc(sizeof(Item));
    if (pNode->pItem == NULL)
        return 3;
    *pNode->pItem = item;
    pNode->left = pNode->right = NULL;
    return 0;
}

void deleteNode(Node *pNode)
{
    if (pNode == NULL)
        return;
    deleteNode(pNode->left);
    deleteNode(pNode->right);
    deleteItem(pNode->pItem);
    free(pNode);
}

typedef Node* Tree;

int push(Tree *pTree, Item item)
{
    if (pTree == NULL)
        return 1;
    if (*pTree == NULL)
    {
        *pTree = (Node*)malloc(sizeof(Node));
        if (*pTree == NULL)
            return 3;
        if (initNode(*pTree, item))
            return 4;
        return 0;
    }
    Node *pNode = *pTree;
    for (;;)
        if (item.freq > pNode->pItem->freq)
            if (pNode->right == NULL)
            {
                pNode->right = (Node*)malloc(sizeof(Node));
                if (pNode->right == NULL)
                    return 3;
                if (initNode(pNode->right, item))
                    return 4;
                break;
            } else
                pNode = pNode->right;
        else
        if (pNode->left == NULL)
        {
            pNode->left = (Node*)malloc(sizeof(Node));
            if (pNode->left == NULL)
                return 3;
            if (initNode(pNode->left, item))
                return 4;
            break;
        } else
            pNode = pNode->left;
    return 0;
}

int pop(Tree *pTree, Item *resItem)
{
    if (pTree == NULL)
        return 1;
    if (resItem == NULL)
        return 2;
    if (*pTree == NULL)
        return 3;
    Node *pNode = *pTree, *prev = NULL;
    while (pNode->left != NULL)
    {
        prev = pNode;
        pNode = pNode->left;
    }
    if (prev == NULL)
        *pTree = (*pTree)->right;
    else
        prev->left = pNode->right;
    pNode->right = NULL;
    *resItem = *pNode->pItem;
    free(pNode->pItem);
    free(pNode);
    return 0;
}

int encode(Item *pItem, int64_t dec, int8_t size)
{
    if (pItem == NULL)
        return 1;
    if (size < 0 || size > sizeof(dec) * 8)
        return 3;
    pItem->code.dec = dec;
    pItem->code.size = size;
    encode(pItem->left, dec << 1, 1 + size);
    encode(pItem->right, (dec << 1) + 1, 1 + size);
    return 0;
}

int huffman(Tree *pTree)
{
    if (pTree == NULL)
        return 1;
    if (*pTree == NULL)
        return 2;
    while ((*pTree)->left != NULL || (*pTree)->right != NULL)
    {
        Item item;
        if (initItem(&item, '\0'))
            return 3;
        item.left = (Item*)malloc(sizeof(Item));
        item.right = (Item*)malloc(sizeof(Item));
        if (item.left == NULL || item.right == NULL)
            return 4;
        if (pop(pTree, item.left))
            return 5;
        if (pop(pTree, item.right))
        {
            free(item.right);
            item.freq = item.left->freq;
        } else
            item.freq = item.left->freq + item.right->freq;
        if (push(pTree, item))
            return 6;
    }
    return 0;
}

int fillCodes(Code *codes, Item *pItem)
{
    if (codes == NULL)
        return 1;
    if (pItem == NULL)
        return  2;
    if (pItem->left == NULL && pItem->right == NULL)
        codes[pItem->ch] = pItem->code;
    fillCodes(codes, pItem->left);
    fillCodes(codes, pItem->right);
    return 0;
}

int fileSize(char *fileName, long *size)
{
    if (fileName == NULL)
        return 1;
    if (size == NULL)
        return 2;
    FILE *file = fopen(fileName, "rb");
    if (file == NULL)
        return 3;
    if (fseek(file, 0, SEEK_END))
        return 4;
    *size = ftell(file);
    if (*size == -1L)
        return 5;
    if (fclose(file))
        return 6;
    return 0;
}

int readFile(Item *ascii, long size, char *fileName)
{
    if (ascii == NULL)
        return 1;
    if (size <= 0)
        return 2;
    if (fileName == NULL)
        return 3;
    FILE *file = fopen(fileName, "rb");
    if (file == NULL)
        return 4;
    size_t i;
    for (i = 0; i < size; i++)
    {
        unsigned char ch;
        if (fread(&ch, sizeof(ch), 1, file) != 1)
            return 5;
        ascii[ch].freq++;
    }
    if (fclose(file))
        return 6;
    return 0;
}

int writeTable(Item *ascii, long size, char *fileName)
{
    if (ascii == NULL)
        return 1;
    if (size <= 0)
        return 2;
    if (fileName == NULL)
        return 3;
    FILE *file = fopen(fileName, "wb");
    if (file == NULL)
        return 4;
    if (fwrite(&size, sizeof(size), 1, file) != 1)
        return 5;
    size_t i;
    short n = 0;
    for (i = 0; i < 256; i++)
        if (ascii[i].freq)
            n++;
    if (fwrite(&n, sizeof(n), 1, file) != 1)
        return 5;
    for (i = 0; i < 256; i++)
        if (ascii[i].freq)
        {
            unsigned char ch = i;
            if (fwrite(&ch, sizeof(ch), 1, file) != 1)
                return 5;
            if (fwrite(&ascii[i].freq, sizeof(ascii[i].freq), 1, file) != 1)
                return 5;
        }
    if (fclose(file))
        return 6;
    return 0;
}

int writeBytes(char *fileInput, long n, Code *codes, char *fileOutput)
{
    if (fileInput == NULL)
        return 1;
    if (n <= 0)
        return 2;
    if (codes == NULL)
        return 3;
    if (fileOutput == NULL)
        return 4;
    FILE *input = fopen(fileInput, "rb");
    if (input == NULL)
        return 5;
    FILE *output = fopen(fileOutput, "ab");
    if (output == NULL)
        return 5;
    size_t i, j, dec = 0, size = 0;
    for (i = 0; i < n; i++)
    {
        unsigned char ch;
        if (fread(&ch, sizeof(ch), 1, input) != 1)
            return 6;
        for (j = 0; j < codes[ch].size; j++)
        {
            size_t shift = codes[ch].size - j - 1;
            dec = (dec << 1) + ((codes[ch].dec & (1 << shift)) >> shift);
        }
        size += codes[ch].size;
        while (size >= 8)
        {
            char byte = 0;
            for (j = 0; j < 8; j++)
            {
                size_t shift = size-- - 1;
                byte = (byte << 1) + ((dec & (1 << shift)) >> shift);
            }
            if (fwrite(&byte, sizeof(byte), 1, output) != 1)
                return 7;
        }
    }
    if (fclose(input))
        return 8;
    if (size > 0)
    {
        dec <<= (8 - size);
        size = 8;
        char byte = 0;
        for (j = 0; j < 8; j++)
        {
            size_t shift = size-- - 1;
            byte = (byte << 1) + ((dec & (1 << shift)) >> shift);
        }
        if (fwrite(&byte, sizeof(byte), 1, output) != 1)
            return 7;
    }
    if (fclose(output))
        return 8;
    return 0;
}

int main()
{
    char fileInput[128], fileOutput[128];

    if (printf("Enter the path to the file: ") < 0)
        return 1;
    if (scanf("%127s", fileInput) != 1)
        return 2;

    if (printf("Enter the path to save: ") < 0)
        return 1;
    if (scanf("%127s", fileOutput) != 1)
        return 2;

    if (strcmp(fileInput, fileOutput) == 0)
        return 12;

    size_t i;
    Item ascii[256];
    for (i = 0; i < 256; i++)
        if (initItem(&ascii[i], i))
            return 3;

    long n = 0;
    if (fileSize(fileInput, &n))
        return 4;

    if (readFile(ascii, n, fileInput))
        return 5;

    if (writeTable(ascii, n, fileOutput))
        return 6;

    Tree tree = NULL;

    for (i = 0; i < 256; i++)
        if (ascii[i].freq)
            if (push(&tree, ascii[i]))
                return 7;

    if (huffman(&tree))
        return 8;

    if (encode(tree->pItem, 0, 0))
        return 9;

    Code codes[256] = {0};
    if (fillCodes(codes, tree->pItem))
        return 10;

    deleteNode(tree);

    if (writeBytes(fileInput, n, codes,  fileOutput))
        return 11;

    return 0;
}
