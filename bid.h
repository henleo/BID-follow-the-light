class Category {
  public:
    Category(char color, int minimum = 1024, int maximum = 0, int mean = 0);
    int minimum;
    int maximum;
    int mean;
    char color;
    bool includes(int);
    bool operator==(const Category);
    bool operator<(const Category);
    bool operator>(const Category);
};

class Motor {
private:
  const int _bufferSize;
  const int _in1;
  const int _in2;
  const int _en;
public:
  Motor(const int in1, const int in2, const int en, int power, int driveDirection, const int bufferSize);
  int driveDirection;
  Category** categories;
  int * sensorBuffer;
  int _power;
  int state;
  int trend;
  int bufferMean;
  Category* category;
  Category* prevCategory;
  void evalutateSensorData(int);
  void changeCategory(Category*);
  void flushBuffer(int);
  int calculateBufferMean();
  int getChange();
  bool colorHasChanged();
  void start();
  void stop();
};
