#ifndef _IMAGE_H
#define _IMAGE_H

unsigned char *ImageDecode(const unsigned char *, int, int *, int *, int);
void ImageFree(void *ptr);

#endif
