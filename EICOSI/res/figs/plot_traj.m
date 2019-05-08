clear
clc
load('fig_traj.mat')

figure

plot(tvec(1:2000,:),xvec(1:2000,1));
hold on
plot(tvec(1:2000,:),xdesvec(1:2000,1));
legend theta theta_{des}
xlabel('Time(s)')
ylabel('theta(rad)')

saveas(gcf,'./fig_traj.epsc')