clear
clc
x = -1:0.01:1;
sAssist = 10;
x0Assist = 0.5;
sStop = 20;
x0Stop = 0.8;
yAssist = 0.5*(tanh(sAssist*(x-x0Assist))+1) + 0.5*(tanh((sAssist*(x+x0Assist)))-1); % addition similar to separate fuzzy rule
yStop = 0.5*(tanh(sStop*(x-x0Stop))+1) + 0.5*(tanh((sStop*(x+x0Stop)))-1); % addition similar to separate fuzzy rule
figure(99)
plot(x,yAssist,x,yStop)
grid on
legend('Assist', 'Stop','Location','NorthWest')