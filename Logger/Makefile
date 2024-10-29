#--컴파일러 변수----
CXX = g++

#--플래그 및 라이브러리 설정----
SIGN_VER_FLAGS = -lssl -lcrypto -L /usr/local/opt/openssl/lib
OPENCV = `pkg-config --cflags --libs opencv4`
LIBS = $(OPENCV) -lcurl -ljsoncpp -g -lpthread
CXXFLAGS = -Wall -Wextra -g -O0

#--폴더 설정----
CONFIG_DIR    = config
SIGN_DIR      = sign
CLIENT_DIR    = VMS_Client
MEDIA_DIR     = media
MK_TREE_DIR   = MK_Tree
INFERENCE_DIR = inference
MATADATA_DIR  = matadata

#--소스 파일 및 오브젝트 파일 설정----
SRC = Logger.cpp \
      $(CONFIG_DIR)/Config.cpp \
      $(SIGN_DIR)/sign.cpp \
      $(CLIENT_DIR)/VMS_Client.cpp \
      $(MEDIA_DIR)/Media.cpp \
      $(MEDIA_DIR)/create_cid.cpp \
      $(MK_TREE_DIR)/MK_Tree.cpp \
      $(INFERENCE_DIR)/inference.cpp

OBJ = Logger.o \
      config/Config.o \
      sign/sign.o \
      VMS_Client/VMS_Client.o \
      media/Media.o \
      media/create_cid.o \
      MK_Tree/MK_Tree.o \
      inference/inference.o

#--기본 타겟----
all: Logger

#--타겟 파일을 생성----
Logger: $(OBJ)
	$(CXX) $(OBJ) -o $@ $(LIBS) $(SIGN_VER_FLAGS) $(CXXFLAGS)

#--개별 소스 파일을 오브젝트 파일로 컴파일하는 규칙----
%.o: %.cpp
	$(CXX) -c $< -o $@ $(CXXFLAGS) $(LIBS)

#--clean 규칙----
clean:
	rm -f $(OBJ) Logger
