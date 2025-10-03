# Average Filter

## 3x3 Result
![alt text](/Image_Processing/Spatial_Filtering/AverageFilter/Image/3x3.png)

## Variabl Filter Size 3x3, 5x5 11x11, 25x25
![alt text](/Image_Processing/Spatial_Filtering/AverageFilter/Image/ResultOfVariablKernelSize.png)

### 왜 필터 크기에 따라 영상이 위 결과처럼 변하는가?

Average Filter는 필터 크기별로 평균을 구해 픽셀값을 주변 픽셀값들과의 평균으로 변경한다.
현재 픽셀 값보다 주변 픽셀의 값들이 낮은 경우 주변픽셀과의 평균값이 현재 픽셀값으로 변경되므로, 본래의 값보다 낮아지게 된다.

또 어떻게 보면 픽셀값이 주변 픽셀로 이동하는 흩뿌려지는 느낌으로 생각할 수 있는데, 이렇게 되면 본래 디테일들이 사라지게 되므로
필터 크기를 키우면 키울수록 디테일이 사라지고 훨씬 더 흐릿해보이는 현상이 발생하게 되는 것이다.ㄴ