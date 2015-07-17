import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.net.Socket;
import java.net.UnknownHostException;

public class Serial {
	
	public static void main(String[] args) {
			//while(port++<255){
			try {
				Socket socket = null;
		
				System.out.print("opening socket ");
					socket = new Socket("192.168.111.129", 81);
					System.out.println("sock opened");
					//socket.close();
					//if (true)continue;
					InputStream sin = socket.getInputStream();
				
				final OutputStream socketOutputStream = socket.getOutputStream();

				new Thread(new Runnable() {
					public void run() {
						String line = "";
						try {
							BufferedReader br = new BufferedReader(new InputStreamReader(System.in));

							String input;

							while ((input = br.readLine()) != null) {
								if (!input.startsWith("r"))
								input += "5";
								
								//System.out.println(input);
								for (int i = 0; i < input.length(); i++) {
									socketOutputStream.write(input.charAt(i));
									System.err.print(input.charAt(i));
									try {
										Thread.sleep(100);
									} catch (InterruptedException e) {
										e.printStackTrace();
									}
									socketOutputStream.flush();

								}
								//System.out.println(line);
							}

						} catch (IOException io) {
							io.printStackTrace();
						}
					}
				}).start();

				new Thread(new Runnable() {
					public void run() {
						String line = "";
						BufferedReader in = null;
						in = new BufferedReader(new InputStreamReader(sin));
						while (true) {
							try {
								if ((line = in.readLine()) != null) {
									System.out.println(":" + line);
								}
							} catch (IOException e) {
								e.printStackTrace();
							}
						}
					}
				}).start();
			} catch (Exception gghe) {
				gghe.printStackTrace();
				System.out.println(" error");
			}
		}
	//}	
		
}