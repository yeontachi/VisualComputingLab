# PSNR
PSNR(Peak Signal-to-Noise Ratio)은 영상의 복원 품질을 측정하는 대표적인 척도이다.
원본 영상과 복원(또는 노이즈 제거 후) 영상 간의 화질 차이 정도를 로그 스케일(db)로 표현한다.

공식은 다음과 같다.
$$
\text{PSNR} = 10 \cdot \log_{10}\left( \frac{MAX_I^2}{MSE} \right)
$$


* $MAX_I$ : 영상의 최대 픽셀 값 (8비트 영상의 경우 255)
* $MSE$ (Mean Squared Error) :
  $$
  MSE = \frac{1}{MN}\sum_{i=0}^{M-1}\sum_{j=0}^{N-1}\big(f(i,j)-g(i,j)\big)^2
  $$
  (두 영상 간의 픽셀 값 차이 제곱의 평균)


PSNR 값이 클수록 두 영상이 유사하다.

단위: 데시벨(db, decibel)

일반적인 해석기준은 다음과 같다.

| PSNR (dB) | 화질 판단           |
| --------- | --------------- |
| > 40 dB   | 거의 동일, 매우 좋은 품질 |
| 30~40 dB  | 품질 양호, 약간 차이 있음 |
| 20~30 dB  | 눈에 띄는 손상 존재     |
| < 20 dB   | 심각한 손상          |


### Gauusian Noise
![alt text](/Image_Processing/Spatial_Filtering/Noise_Filtering/PSNR/Images/GaussianNoise.png)

### Salt and Pepper Noise
![alt text](/Image_Processing/Spatial_Filtering/Noise_Filtering/PSNR/Images/SaltAndPepper.png)

### Adaptive Median Filter
![alt text](/Image_Processing/Spatial_Filtering/Noise_Filtering/PSNR/Images/AdaptiveMedianFilter.png)

### Geometric Mean Filter
![alt text](/Image_Processing/Spatial_Filtering/Noise_Filtering/PSNR/Images/GeometricMeanFilter.png)

### Median Filter
![alt text](/Image_Processing/Spatial_Filtering/Noise_Filtering/PSNR/Images/MedianFilter.png)