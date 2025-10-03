# Gaussian Filter

## 3x3 result
![alt text](/Image_Processing/Spatial_Filtering/GaussianFilter/results/Images/3x3.png)

## 7x7 reuslt
![alt text](/Image_Processing/Spatial_Filtering/GaussianFilter/results/Images/7x7.png)

## conclusion
먼저 가우시안 필터를 적용할 때, sigma 값이 커질 수록 영상이 블러 처리가 되는 느낌을 받는다.
그 이유는 가우시안 필터는 현재 여기서 적용된 가우시안은 고주파 성분을 부드럽게 제거하기 때문이다. 
즉 시그마가 커질수록 주파수가 중심에서 부드럽게 멀어지므로, sigma값을 크게 적용하면 블러링이 되는 것이다.

커널 크기가 커지면 그 폭이 커지므로 더 큰 효과를 얻을 수 있게된다.


