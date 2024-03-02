# 可执行文件名
TARGET = multiThreadDownload

# 编译器
CXX = g++
# 编译选项
CXXFLAGS = -std=c++11 -Wall -lcurl -lpthread

SRCDIR = .
SRCS = $(wildcard $(SRCDIR)/*.cpp)
OBJS = $(patsubst %.cpp, %.o, $(SRCS))
RM = rm -f

# 默认目标
all: $(TARGET)

# 生成可执行文件
$(TARGET): $(OBJS)
	$(CXX) $^ -o $@ $(CXXFLAGS)

# 编译目标文件
%.o: %.cpp
	$(CXX) -g -c $< -o $@ $(CXXFLAGS)

# 清理生成文件
clean:
	$(RM) $(TARGET) $(OBJS)
