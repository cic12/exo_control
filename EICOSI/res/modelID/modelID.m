clear
clc

%% Read current results
addpath('C:\Users\prince\Dropbox\PhD\Code\EICOSI\EICOSI\res')

t = dlmread('tvec.txt',',');
u = importfile('uvec.txt');
x = dlmread('xvec.txt',',');

%% Process and save
u = u/68/2.5;
x = x(:,1);
dt = t(2)-t(1);

[bLPx,aLPx] = butter(4,10/500,'low');
x = filtfilt(bLPx,aLPx,x);

dx = zeros(length(x),1);
dx(2:end) = diff(x)/dt;

ddx = zeros(length(dx),1);
ddx(2:end) = diff(dx)/dt;

sgnDx = sign(dx);

sinx = sin(x);

% Linear least squares
span = 1:length(x);
C = [ddx(span) dx(span) sgnDx(span) sinx(span)];
d = u(span);
LB = [0 0 0 0]';
UB = [1 4 4 10]';
params = lsqlin(C,d,[],[],[],[],LB, UB); % J B A tau_g

save('testID.mat')

%% Plot results
figure(1)
subplot(6,1,1)
plot(x)
legend x
subplot(6,1,2)
plot(dx)
legend dx
subplot(6,1,3)
plot(ddx)
legend ddx
subplot(6,1,4)
plot(sinx)
legend sinx
subplot(6,1,5)
plot(sgnDx)
legend sgnDx
subplot(6,1,6)
plot(u)
legend u

figure(2);
clf
plot(d); hold on
plot(C*params)

disp(params)