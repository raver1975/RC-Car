import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.net.Socket;
import java.net.UnknownHostException;

public class Serial {

	private static InputStream sin;

	public static void main(String[] args) {
		// while(port++<255){

		try {
			Socket socket = null;

			System.out.print("opening socket ");
			socket = new Socket("192.168.0.15", 81);
			System.out.println("sock opened");
			// socket.close();
			// if (true)continue;
			sin = socket.getInputStream();

			final OutputStream socketOutputStream = socket.getOutputStream();

			new Thread(new Runnable() {
				private String last="0";

				public void run() {
					String line = "";
					try {
						BufferedReader br = new BufferedReader(new InputStreamReader(System.in));

						String input;

						while ((input = br.readLine()) != null) {
//							if (!input.startsWith("r"))
								//input += "5";
							if (input==null||input.isEmpty()||input.equals("")){input=last;}
							else last=input;

							// System.out.println(input);
							for (int i = 0; i < input.length(); i++) {
								socketOutputStream.write(input.charAt(i));
								System.err.print(input.charAt(i));
//								try {
//									Thread.sleep(300);
//								} catch (InterruptedException e) {
//									e.printStackTrace();
//								}
								socketOutputStream.flush();

							}
							// System.out.println(line);
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
							while (true) {
								System.out.println(" error");

								Socket socket = null;
								System.out.print("opening socket ");
								try {
									socket = new Socket("192.168.111.129", 81);
								} catch (UnknownHostException e2) {
									e.printStackTrace();
								} catch (IOException e2) {
									e.printStackTrace();
								}
								if (socket != null) {
									System.out.println("sock opened");
									try {
										sin = socket.getInputStream();
									} catch (IOException e2) {
										e.printStackTrace();
										
									}
								}
								try {
									Thread.sleep(1000);
								} catch (InterruptedException e2) {
									e.printStackTrace();
								}
							}
						}
					}
				}
			}).start();
		} catch (Exception gghe) {
			while (true) {
				System.out.println(" error");
				Socket socket = null;

				System.out.print("opening socket ");
				try {
					socket = new Socket("192.168.111.129", 81);
				} catch (UnknownHostException e) {
					//e.printStackTrace();
				} catch (IOException e) {
					//e.printStackTrace();
				}
				if (socket != null) {
					System.out.println("sock opened");
					try {
						sin = socket.getInputStream();
					} catch (IOException e) {
						e.printStackTrace();
					}
					break;
				}
				try {
					Thread.sleep(1000);
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
			}
		}
	}
}