function Level_0L()

FileName = 'Book1.csv';
Data = csvread(FileName, 0, 0);
Data = Data';
meanData = mean(Data);
maxData = max(Data);
minData = min(Data);
disp(['max value = ' num2str(maxData)]);
disp(['min value = ' num2str(minData)]);

plot(Data);
grid on;
hold on;
plot(1:numel(Data), meanData, '--rs',...
    'LineWidth',0.5,...
    'MarkerSize',10,...
    'MarkerEdgeColor','r',...
    'MarkerFaceColor',[0.5,0.5,0.5]);
xlabel('Time')
ylabel('Random value')
