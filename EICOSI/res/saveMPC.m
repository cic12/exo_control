clear
clc

addpath('./data')

%% Import
tvec = dlmread('res/tvec.txt',',');
% uvec = importfile('res/uvec.txt');
uvec = dlmread('res/uvec.txt');
xvec = dlmread('res/xvec.txt',',');
xdesvec = dlmread('res/xdesvec.txt',',');
ai = dlmread('ai.txt',',');
mode = dlmread('mode.txt',',');

save(strcat('data/',datestr(now,'yyyymmdd_HHMMSS'),'.mat'))

%% Plot
plotMPC
% xmgMPC