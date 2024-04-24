clc
clear 
close all
addpath(genpath('helperFunctions'));
addpath('howeCoreProcedures');
Image_dir = 'demoImages';
listing = cat(1, dir(fullfile(Image_dir, '*.bmp')));
% The final output will be saved in this directory:
result_dir = fullfile(Image_dir, 'result');
% Preparations for saving results.
if ~exist(result_dir, 'dir'), mkdir(result_dir); end

for i_img = 1:length(listing)
    % Input = imread(fullfile(Image_dir,listing(i_img).name));
    BinarizeVisibility(fullfile(Image_dir,listing(i_img).name), fullfile(result_dir,listing(i_img).name))
end

function BinarizeVisibility (origImageFileName, outputImageFileName)
    % For DEBUGGING AND DEMOING
%     origImageFileName = 'demoImages\dibco09_HW4_in.bmp';
%     outputImageFileName = 'demoImages\dibco09_HW4_bin.bmp';

    %setting parameters to required values
    
    %transformation variables
    variables = struct('pointOfView', [0.5,0.5,0.5], 'transformationType', 'linear',...
       'a1', 1, 'a0', 0.05);

   %HPR and TPO variables
%   compromise - best occlusion visibility performance parameters:
%   (1) gamma=0.01  for the Dibco hand-written dataset, 
%   (2) gamma=0.005  for the Dibco printed dataset, 
%   (3) gamma=0.001 for the Bickley dataset
    variables.kernelType = 'exponential';
    variables.gammaVis=-0.5;
    variables.gammaOcc= 0.005; 
    
    %s&p noise filtering variables
    variables.filterVec = [0.1, 0.4, 1, 0.4, 0.1];
    variables.filterThreshold = 0.54;

    %reading the image
    origImg = imread(origImageFileName); %SHOULD this actually be like this???
    if size(origImg, 3)==3
        rgbImg = origImg;
        grayImg=rgb2gray(rgbImg);
    else
        grayImg=origImg;
    end
    
    %calculating the lowlights map
    lowlights = createLowLightsMap(grayImg, variables);
        
    %applying the Howe framework to the map - currently set to howeV1.
    %could be changed according to image
    binImage = ~binarizeImageV1(lowlights);
%     binImageAlg1 = ~binarizeImageAlg1(lowlights);
%     binImageAlg3 = ~binarizeImageAlg3(lowlights);

%   figure; imshow(binImage); title('binarized');

    %s&p noise removal
    cleanBinImage = cleanSPnoise(binImage, variables); 
    
    % writing the image
    imwrite(cleanBinImage, outputImageFileName); 

end

