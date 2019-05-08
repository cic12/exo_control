clear
clc
load('mmg_torque_contract_processed.mat')
% load('mmg_torque_extend_processed.mat')
% load('mmg_torque_swing_processed.mat')

x5 = data.mmg(:,1);
x6 = data.mmg(:,2);
tvec = 0.001:0.001:length(data.mmg)/1000;

% Low Pass
alpha = 0.005; % low means more smooth
n = length(x5);
x5lp = zeros(n,1);
x6lp = zeros(n,1);
for i = 1:length(x5)-1
    x5lp(i+1) = (1-alpha)*x5lp(i) + alpha*x5(i);
    x6lp(i+1) = (1-alpha)*x6lp(i) + alpha*x6(i);
end

a = 10; % input scaling
b = 1000; % steepness
c = 0.4; % crossing point

x = 0:0.0001:0.1;
y = sigmf(x*a,[b c]);
m1 = sigmf(x5*a,[b c]);
m2 = sigmf(x6*a,[b c]);
m1lp = sigmf(x5lp*a,[b c]);
m2lp = sigmf(x6lp*a,[b c]);

figure(2)
clf

subplot(4,1,1)
plot(x,y)
grid on

subplot(4,1,2)
plot(tvec,x5)
hold on
plot(tvec,x6)
plot(tvec,x5lp)
plot(tvec,x6lp)
grid on
legend x5 x6 x5lp x6lp

subplot(4,1,3)
plot(tvec,m1)
hold on
plot(tvec,m2)
grid on
legend m1 m2

subplot(4,1,4)
plot(tvec,m1lp)
hold on
plot(tvec,m2lp)
grid on
legend mlp m2lp