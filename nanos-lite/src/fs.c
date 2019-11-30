#include "fs.h"

typedef size_t (*ReadFn) (void *buf, size_t offset, size_t len);
typedef size_t (*WriteFn) (const void *buf, size_t offset, size_t len);



typedef struct {
  char *name;
  size_t size;
  size_t disk_offset;
  size_t open_offset;
  ReadFn read;
  WriteFn write;
} Finfo;



enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB};

size_t invalid_read(void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

size_t invalid_write(const void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  {"stdin", 0, 0, 0, invalid_read, invalid_write},
  {"stdout", 0, 0, 0, invalid_read, serial_write},
  {"stderr", 0, 0, 0, invalid_read, serial_write},
#include "files.h"
};


#define NR_FILES (sizeof(file_table) / sizeof(file_table[0]))

void init_fs() {
  // TODO: initialize the size of /dev/fb
}


int fs_open(const char *pathname, int flags, int mode){
  //printf("open\n");
  for(int i=0;i<NR_FILES;i++){
    if(strcmp(file_table[i].name,pathname)==0){
      file_table[i].open_offset=0;
      //printf("%d\n",i);
      return i;
    }
  }
  printf("file %s does not exist!",pathname);
  assert(0);
  return -1;
}

size_t fs_read(int fd,void *buf,size_t len){
  size_t read = -1;
  switch(fd){
    case FD_STDIN:
    case FD_STDOUT:
    case FD_STDERR:read=0;break;
    default:{
      assert(0<=fd && fd<NR_FILES);
      Finfo *f=&file_table[fd];
      read = (f->open_offset+len>f->size) ? (f->size-f->open_offset):len;
      ramdisk_read(buf,f->disk_offset+f->open_offset,read);
      f->open_offset+=read;
      break;
    }
  }

  return read;
}

int fs_close(int fd){
  return 0;
}

size_t fs_offset(int fd){
  return file_table[fd].disk_offset;
}

size_t fs_write(int fd,const void *buf,size_t len){
  size_t write=-1;
  switch(fd){
    case FD_STDOUT:
    case FD_STDERR:{
      for(int i=0;i<len;i++){
        _putc(((char *)buf)[i]);
      }
      write=len;
      break;
    }
    default:{
      assert(0<=fd && fd<NR_FILES);
      Finfo *f = &file_table[fd];
      write = (f->open_offset+len > f->size) ? (f->size-f->open_offset) : len;
      ramdisk_write(buf,f->disk_offset+f->open_offset,write);
      f->open_offset+=write;
      break;
    }
  }
  return write;
}

size_t fs_lseek(int fd,size_t offset,int whence){
  if(fd==FD_STDIN||fd==FD_STDOUT||fd==FD_STDERR){
    assert(0);
    return 0;
  }
  Finfo *f = &file_table[fd];
  size_t result=-1;
  switch(whence){
    case SEEK_SET:{
      if(0<=offset && offset<=f->size){
        f->open_offset = offset;
        result=f->open_offset;
      }
      break;
    }
    case SEEK_CUR:{
      if((offset+f->open_offset>=0)&&(offset+f->open_offset<=f->size)){
        f->open_offset+=offset;
        result = f->open_offset;
      }
      break;
    }
    case SEEK_END:{
      if((offset+f->size>=0)&&(offset+f->size<=f->size)){
        f->open_offset = f->size+offset;
        result = f->open_offset;
      }
      break;
    }
  }
  return result;
}