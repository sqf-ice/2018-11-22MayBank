CROSS_COMPILE=arm-unknown-linux-gnu-

CC=$(CROSS_COMPILE)gcc
RM=rm
CP=cp
MKUPT=upt

# Application directory definition
BINDIR = bin
SRCDIR = src
INCDIR = inc
OBJDIR = obj_err
LIBDIR = lib
LIBINCDIR = libapi

# Module directory definition
PRINTINCDIR = module/libprint/inc
PRINTSRCDIR = module/libprint/src
SECURITYIINCDIR = module/libsecurity/inc
SECURITYISRCDIR = module/libsecurity/src
CARDINCDIR = module/libcard/inc
CARDSRCDIR = module/libcard/src
UIINCDIR = module/libui/inc
UISRCDIR = module/libui/src
TOOLINCDIR = module/libtool/inc
TOOLSRCDIR = module/libtool/src
COMMINCDIR = module/libcomm/inc
COMMSRCDIR = module/libcomm/src
TMSINCDIR = module/libtms/inc
TMSSRCDIR = module/libtms/src


# SDK directory definition
SDKDIR = C:\\Phoenix_SDK
SDKAPIDIR = $(SDKDIR)\\api\\gcc_4.2.0
SDKAPIVER = 7.0.0
SDKAPIINC = $(SDKAPIDIR)\\$(SDKAPIVER)\\inc
SDKAPILIB = $(SDKAPIDIR)\\$(SDKAPIVER)\\lib
SDKSYSLIBLIB = $(SDKDIR)\\tools\\cross-compiler\\gcc_4.2.0\\arm-unknown-linux-gnu\\sys-root\\usr\\lib

CFLAGS += -Wall -DNDEBUG $(CROSS_CFLAGS) -O2 $(INCPATH) -D DEMO -D USE_TMS

# File search path
VPATH = src $(PRINTSRCDIR) $(SECURITYISRCDIR) $(CARDSRCDIR) $(UISRCDIR) $(TOOLSRCDIR) $(COMMSRCDIR)  $(TMSSRCDIR) $(OBJDIR)

# Header file search path
INCLPATH = -I$(INCDIR) -I$(PRINTINCDIR) -I$(SECURITYIINCDIR) -I$(CARDINCDIR) -I$(UIINCDIR) -I$(TOOLINCDIR) -I$(COMMINCDIR) -I$(TMSINCDIR) -I$(LIBINCDIR) -I$(SDKAPIINC)

# Program link parameters
LDFLAGS += -L$(SDKAPILIB) -lndk -lpthread
LDFLAGS += -L$(SDKSYSLIBLIB) -lssl -lcrypto 
LDFLAGS += -L$(LIBDIR) -lemv -ldl -lmxml -lcurl
LDFLAGS += -L$(LIBDIR) -lentrypoint -lpaywave -lpaypass -lqpboc -lexpresspay -ljcb -lpure
LDFLAGS += -lm
# Generated program name
NAME = main
APPNAME = MAYBANK.NLD

# Module used in the program
SRCS = $(wildcard $(SRCDIR)/*.c $(PRINTSRCDIR)/*.c $(SECURITYISRCDIR)/*.c $(CARDSRCDIR)/*.c $(UISRCDIR)/*.c $(TOOLSRCDIR)/*.c $(COMMSRCDIR)/*.c $(TMSSRCDIR)/*.c)
SRSS = $(notdir $(SRCS))
OBJS = $(subst .c,.o,$(SRSS))
OBJSD = $(addprefix $(OBJDIR)/,$(SRSS))
OBJSY = $(subst .c,.o,$(OBJSD))

# Include dependency file
all: NLD

$(NAME):config $(OBJS)
	$(CC) $(CFLAGS) -o $(BINDIR)/$(NAME) $(OBJSY) $(LDFLAGS)
%.o: %.c
	$(CC) -c $(CFLAGS) $(INCLPATH) $< -o $(OBJDIR)/$@
# Automatic generation of dependency files  
config: $(subst .o,.deps, $(OBJS))

%.deps: %.c
	$(CC) -MM $(INCLPATH) $< >$(OBJDIR)/$@ 

.PHONY:clean
clean:
	-$(RM) $(BINDIR)/$(NAME)
	-$(RM) $(OBJDIR)/*.o
	-$(RM) $(OBJDIR)/*.deps
	
NLD:$(NAME)
	$(MKUPT) -h $(BINDIR)/filepath.txt -p $(BINDIR)/param.ini -o $(BINDIR)/$(APPNAME)	
