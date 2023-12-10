import android.os.AsyncTask;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;

import androidx.appcompat.app.AppCompatActivity;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.Socket;

public class MainActivity extends AppCompatActivity {

    private EditText messageEditText;
    private TextView responseTextView;

    private static final String SERVER_IP = "88.201.132.38"; 
    private static final int SERVER_PORT = 12345; 

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // Инициализация элементов пользовательского интерфейса
        messageEditText = findViewById(R.id.messageEditText);
        responseTextView = findViewById(R.id.responseTextView);
        Button sendButton = findViewById(R.id.sendButton);

        // Установка слушателя событий для кнопки отправки
        sendButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                // Получаем текст
                String message = messageEditText.getText().toString();

                // Создание и запуск асинхронной задачи для отправки сообщения на сервер
                SendMessageTask sendMessageTask = new SendMessageTask();
                sendMessageTask.execute(message);
            }
        });
    }

    // Класс для выполнения сетевых операций в фоновом режиме
    private class SendMessageTask extends AsyncTask<String, Void, String> {

        @Override
        protected String doInBackground(String... params) {
            // Получение сообщения для отправки
            String message = params[0];
            String response;

            try {
                // Установка соединения с сервером
                Socket socket = new Socket(SERVER_IP, SERVER_PORT);

                // Получение потоков ввода/вывода для обмена данными с сервером
                PrintWriter out = new PrintWriter(socket.getOutputStream(), true);
                BufferedReader in = new BufferedReader(new InputStreamReader(socket.getInputStream()));

                // Отправка сообщения на сервер и получение ответа
                out.println(message);
                response = in.readLine();

                socket.close();
            } catch (IOException e) {
                e.printStackTrace();
                response = "Ошибка при отправке/получении сообщения";
            }

            // Возвращение ответа для onPostExecute
            return response;
        }

        @Override
        protected void onPostExecute(String result) {
            // Обновление UI с полученным ответом от сервера
            responseTextView.setText(result);
        }
    }
}
