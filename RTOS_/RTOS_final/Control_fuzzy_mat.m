clc;
clear UART_arduino;

% Asumiendo que ya tienes RTOS cargado en Workspace
% Y RTOS tiene 2 entradas: error_actual y humedad

% Configura puerto serial con la misma velocidad que Arduino
UART_arduino = serialport("COM11",115200);
configureTerminator(UART_arduino,"LF");
flush(UART_arduino);

dato_error_previo = 0;
dato_humedad_previa = 0;  % no usado aquí, pero lo tienes
th_des = 25;

while true
    try
        linea = readline(UART_arduino);
        valores = split(linea, ',');

        if numel(valores) < 2
            continue
        end

        humedad = str2double(valores(1));
        error_actual = str2double(valores(2));

        if isnan(humedad) || isnan(error_actual)
            continue
        end

        % --- CONTROL DIFUSO MAMDANI ---
        % Solo las 2 entradas que el sistema fuzzy espera
        input_vector = [humedad error_actual];  

        output = evalfis(RTOS, input_vector);

        % Guardar valores previos si los necesitas para otra cosa
        dato_error_previo = error_actual;
        dato_humedad_previa = humedad;

        % Para resistencia, solo ON/OFF (0 o 1)
        pwm_res = output(1) > 0.1;

        % Ventilador PWM entre 0 y 255, asegurado en rango
        pwm_vent = round(max(min(output(2),255),0));

        writeline(UART_arduino, sprintf('%d,%d', pwm_res, pwm_vent));

        pause(0.05);

    catch ME
        disp("Error:");
        disp(ME.message);
        break
    end
end

clear UART_arduino
