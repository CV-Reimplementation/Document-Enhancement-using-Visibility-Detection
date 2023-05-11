clc
clear 
close all
addpath(genpath('helperFuncs'));
addpath('coreShadowingFuncs');
Image_dir = 'demoImages';
listing = cat(1, dir(fullfile(Image_dir, '*.jpg')));
% The final output will be saved in this directory:
result_dir = fullfile(Image_dir, 'result');
% Preparations for saving results.
if ~exist(result_dir, 'dir'), mkdir(result_dir); end

for i_img = 1:length(listing)
    % Input = imread(fullfile(Image_dir,listing(i_img).name));
    unshadowVisibility(fullfile(Image_dir,listing(i_img).name), fullfile(result_dir,listing(i_img).name))
end


function unshadowVisibility (origImageFileName, outputImageFileName)

% For DEBUGGING AND DEMOING
%     origImageFileName = 'demoImages\natiR36_in.png';
%     outputImageFileName = 'demoImages\natiR36_outVisWB.png';

    %setting parameters to required values

    optionalGammaValues = [0.007,0.009,0.01,0.04];
    visibilityParams = struct('kernelType', 'exponential', 'visDetection', 0, 'occDetection', 1, ...
        'viewPoint', [0.5, 0.5, 1], 'sphericalTransformationType', 'linear', 'a0', .5, 'a1', 1, 'gammaVis', 0, 'gammaOcc', optionalGammaValues(3));
    unshadowingParams = struct('interpType', 'natural', 'intClusterNum', 4);
    filteringParams = struct('maskGauss', 7, 'enhanceRadius', 2, 'enhanceAmount', .5, 'imgGauss', 0, 'imgMedWindow', 3);
    whiteBalancingParams = struct('wbAlg', 'robust', 'wbClusterNum', 4);

    visImgStruct = createVisibilityStruct(origImageFileName, visibilityParams);
    
    unshadowedStruct = createUnshadowedStruct(visImgStruct, unshadowingParams, filteringParams);
    
    [weightedWB, originalWB] = whiteBalanceImg(unshadowedStruct, whiteBalancingParams);
    
    imwrite(weightedWB, outputImageFileName);
end
