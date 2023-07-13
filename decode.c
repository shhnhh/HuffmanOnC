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

int readTable(Item *ascii, char *fileName)
{
    if (ascii == NULL)
        return 1;
    if (fileName == NULL)
        return 2;
    FILE *file = fopen(fileName, "rb");
    if (file == NULL)
        return 3;
    long size;
    if (fread(&size, sizeof(size), 1, file) != 1)
        return 4;
    short n = 0;
    if (fread(&n, sizeof(n), 1, file) != 1)
        return 4;
    size_t i;
    for (i = 0; i < n; i++)
    {
        unsigned char ch;
        unsigned long freq;
        if (fread(&ch, sizeof(ch), 1, file) != 1)
            return 4;
        if (fread(&freq, sizeof(freq), 1, file) != 1)
            return 4;
        ascii[ch].freq = freq;
    }
    if (fclose(file))
        return 5;
    return 0;
}

int decode(Tree tree, char *fileInput, char *fileOutput)
{
    if (tree == NULL)
        return 1;
    if (fileInput == NULL)
        return 2;
    if (fileOutput == NULL)
        return 3;
    FILE *input = fopen(fileInput, "rb");
    if (input == NULL)
        return 4;
    long size = 0;
    if (fread(&size, sizeof(size), 1, input) != 1)
        return 5;
    short n = 0;
    if (fread(&n, sizeof(n), 1, input) != 1)
        return 5;
    Item none;
    if (fseek(input, n * (sizeof(none.ch) + sizeof(none.freq)), SEEK_CUR))
        return 6;
    FILE *output = fopen(fileOutput, "wb");
    if (output == NULL)
        return 4;
    size_t i, j = 0;
    unsigned char ch;
    Item *pItem = tree->pItem;
    for (i = 0; j < size; i++)
    {
        if (pItem->left == NULL && pItem->right == NULL)
        {
            j++;
            if (fwrite(&pItem->ch, sizeof(pItem->ch), 1, output) != 1)
                return 7;
            pItem = tree->pItem;
        }
        if (i % 8 == 0)
            if (fread(&ch, sizeof(ch), 1, input) != 1)
                return 5;
        int bit = (ch >> (7 - i % 8)) & 1;
        if (bit == 0 && pItem->left != NULL)
            pItem = pItem->left;
        else if (bit == 1 && pItem->right != NULL)
            pItem = pItem->right;
    }
    if (fclose(input))
        return 8;
    if (fclose(output))
        return 8;
    return 0;
}

int main()
{
    char fileInput[128], fileOutput[128] = "recovered";

    if (printf("Enter the path to the file: ") < 0)
        return 1;
    if (scanf("%127s", fileInput) != 1)
        return 2;

    if (printf("Enter the path to save: ") < 0)
        return 1;
    if (scanf("%127s", fileOutput) != 1)
        return 2;

    if (strcmp(fileInput, fileOutput) == 0)
        return 9;

    size_t i;
    Item ascii[256];
    for (i = 0; i < 256; i++)
        if (initItem(&ascii[i], i))
            return 3;

    if (readTable(ascii, fileInput))
        return 4;

    Tree tree = NULL;

    for (i = 0; i < 256; i++)
        if (ascii[i].freq)
            if (push(&tree, ascii[i]))
                return 5;

    if (huffman(&tree))
        return 6;

    if (encode(tree->pItem, 0, 0))
        return 7;

    if (decode(tree, fileInput, fileOutput))
        return 8;

    deleteNode(tree);

    return 0;
}
