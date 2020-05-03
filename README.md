# PinDetection
This is a fast detection algorithm based on traditional machine learning for pin defects of SMD components, which locates the SMD components and pins in turn, estimates the pin area, and then judges whether the pins are defective. The algorithm is able to identify pin defects accurately, and the average detection speed is 61.0610ms
In the PinDetection.h file, we define the global constant, and create the pindetection class, 
which contains Otsu function, horizontal, vertical projection function, scale function and other private functions,
as well as public functions such as loading image and device connection.
In the PinDetection.cpp file, we import the PinDetection.h head file. Otsu function is constructed to select binary threshold of image.
The horizontal and vertical projection functions are constructed to 
obtain the number of pixels in each column and row in the horizontal and vertical directions of the image. 
The SMD component location function is constructed to realize the location function. 
In order to realize the function of judging whether pins are connected or not, a pin discrimination function is constructed. 
The scale coefficient function is constructed to obtain the ratio K between the area of the actual pin and
the area of the smallest external rectangle of the pin in the image.
The function of pin area estimation is constructed to realize the function of pin area calculation.
In the main.cpp file, we call the function of opening the device, loading the image function, 
SMD component positioning function, pin defect discrimination function and pin area calculation function in turn.
