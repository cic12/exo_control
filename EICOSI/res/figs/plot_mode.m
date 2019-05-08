clear
clc
load('fig_mode.mat')

figure(1)
clf

subplot(4,1,1)
plot(tvec(1:2000,:),xvec(1:2000,1));
hold on
plot(tvec(1:2000,:),xdesvec(1:2000,1));
legend theta theta_{des}
xlabel('Time(s)')
ylabel('theta(rad)')

subplot(4,1,2)
plot(tvec(1:2000,:),xvec(1:2000,5));
xlabel('Time(s)')
ylabel('emg(V)')
ylim([-0.025 0.025])
legend emg1
grid on

subplot(4,1,3)
plot(tvec(1:2000,:),xvec(1:2000,6));
xlabel('Time(s)')
ylabel('emg(V)')
ylim([-0.025 0.025])
legend emg2
grid on

subplot(4,1,4)
plot(tvec(1:2000,:),uvec(1:2000,1));
xlabel('Time(s)')
ylabel('u(N)')
ylim([-10 10])
legend u
grid on

set(gcf,'Position',[680   200   560   420+180])

saveas(gcf,'./fig_mode.epsc')