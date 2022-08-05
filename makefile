
.PHONY : all clean rebuild

KERNEL_SRC := 	kmain.c 	\
				screen.c 	\
				kernel.c 	\
				utility.c 	\
				task.c 		\
				interrupt.c \
				ihandler.c	\
				list.c		\
				queue.c

KERNEL_ADDR := B000
IMG := LightOS
IMG_PATH := /mnt/hgfs

DIR_DEPS := deps
DIR_EXES := exes
DIR_OBJS := objs

DIRS := $(DIR_DEPS) $(DIR_EXES) $(DIR_OBJS)

KENTRY_SRC := kentry.asm
BLFUNC_SRC := blfunc.asm
BOOT_SRC   := boot.asm
LOADER_SRC := loader.asm
COMMON_SRC := common.asm

BOOT_OUT   := boot
LOADER_OUT := loader
KERNEL_OUT := kernel
KENTRY_OUT := $(DIR_OBJS)/kentry.o

EXE := kernel.out
EXE := $(addprefix $(DIR_EXES)/, $(EXE))

SRCS := $(KERNEL_SRC)
# SRCS := $(wildcard *.c)
OBJS := $(SRCS:.c=.o)
OBJS := $(addprefix $(DIR_OBJS)/, $(OBJS))
DEPS := $(SRCS:.c=.dep)
DEPS := $(addprefix $(DIR_DEPS)/, $(DEPS))

all : $(DIR_OBJS) $(DIR_EXES) $(IMG) $(BOOT_OUT) $(LOADER_OUT) $(KERNEL_OUT)
	@echo "Build Success ==> LightOS!"
	
ifeq ("$(MAKECMDGOALS)", "all")
-include $(DEPS)
endif

ifeq ("$(MAKECMDGOALS)", "")
-include $(DEPS)
endif

$(IMG) :
	bximage $@ -q -fd -size=1.44
	
$(BOOT_OUT) : $(BOOT_SRC) $(BLFUNC_SRC)
	nasm $< -o $@
	dd if=$@ of=$(IMG) bs=512 count=1 conv=notrunc
	
$(LOADER_OUT) : $(LOADER_SRC) $(COMMON_SRC) $(BLFUNC_SRC)
	nasm $< -o $@
	sudo mount -o loop $(IMG) $(IMG_PATH)
	sudo cp $@ $(IMG_PATH)/$@
	sudo umount $(IMG_PATH)
	
$(KENTRY_OUT) : $(KENTRY_SRC) $(COMMON_SRC)
	nasm -f elf $< -o $@
    
$(KERNEL_OUT) : $(EXE)
	chmod 777 elf2kobj
	sudo ./elf2kobj -c$(KERNEL_ADDR) $< $@
	sudo mount -o loop $(IMG) $(IMG_PATH)
	sudo cp $@ $(IMG_PATH)/$@
	sudo umount $(IMG_PATH)
	
$(EXE) : $(KENTRY_OUT) $(OBJS)
	ld -s $^ -o $@
	
$(DIR_OBJS)/%.o : %.c
	gcc -fno-builtin -fno-stack-protector -c $(filter %.c, $^) -o $@

$(DIRS) :
	mkdir $@

ifeq ("$(wildcard $(DIR_DEPS))", "")
$(DIR_DEPS)/%.dep : $(DIR_DEPS) %.c
else
$(DIR_DEPS)/%.dep : %.c
endif
	@echo "Creating $@ ..."
	@set -e; \
	gcc -MM -E $(filter %.c, $^) | sed 's,\(.*\)\.o[ :]*,objs/\1.o $@ : ,g' > $@
	
clean :
	sudo rm -fr $(IMG) $(BOOT_OUT) $(LOADER_OUT) $(KERNEL_OUT) $(DIRS)
	
rebuild :
	@$(MAKE) clean
	@$(MAKE) all
