#! /usr/bin/python3
# Parse the XML file and return important information
# Usage: python3 analysis4.py <path to xml file>
from xml.etree import ElementTree as ET
import sys
import matplotlib.pyplot as plt

et = ET.parse(sys.argv[1])

totalClients = 0
lostClients = set()  # Use a set to avoid duplicate addresses
clientCount = 0  # Số lượng khách hàng đã tính toán
lostClientsRatios = []  # Lưu trữ tỉ lệ khách hàng bị mất kết nối cho mỗi node

# Loop through each flow in flow stats
for flow in et.findall("FlowStats/Flow"):
    flowId = flow.get("flowId")

    # Find the flow with the same id in Ipv4FlowClassifier
    for ipv4flow in et.findall("Ipv4FlowClassifier/Flow"):
        if ipv4flow.get("flowId") == flowId:
            break
    # Get the source and destination address and port
    srcAdd = ipv4flow.get("sourceAddress")
    srcPrt = ipv4flow.get("sourcePort")
    desAdd = ipv4flow.get("destinationAddress")
    desPrt = ipv4flow.get("destinationPort")
    print(f"Flow {flowId} ({srcAdd}/{srcPrt} --> {desAdd}/{desPrt}): ")
    if srcPrt != "9":
        totalClients += 1
    # Get total transmitted and received bytes of the flow
    txBytes = float(flow.get("txBytes"))
    rxBytes = float(flow.get("rxBytes"))

    # Get the number of transmitted and received packets
    txPackets = int(flow.get("txPackets"))
    rxPackets = int(flow.get("rxPackets"))
    print(f"\tNumber of transmitted packets: {txPackets}")
    print(f"\tNumber of received packets: {rxPackets}")

    # Calculate the mean delay
    delaySum = float(flow.get("delaySum")[:-2])
    delayMean = None if rxPackets == 0 else delaySum / rxPackets * 1e-9
    print("\tMean Delay: " + (f"{delayMean * 1e3:.2f} ms" if delayMean else "None"))

    # Calculate the packet loss ratio
    lostPackets = int(flow.get("lostPackets"))
    if lostPackets != 0:
        lostClients.add(srcAdd)
    packetLossRatio = (txPackets - rxPackets) / txPackets * 100
    print(f"\tPacket Loss Ratio: {packetLossRatio:.2f} %")

    clientCount += 1
    if clientCount >= 2:  # Tính toán từ node 2 trở lên
        lostClientRatio = len(lostClients) / totalClients * 100
        lostClientsRatios.append(lostClientRatio)
        print(f"Lost Clients Ratio (Node {clientCount}): {lostClientRatio:.2f}% ({len(lostClients)}/{totalClients})")

    if clientCount == 30:  # Đạt đến node thứ 30
        break

print(f"Lost clients: {list(lostClients)}")

# Plot biểu đồ Lost Clients Ratio
plt.figure(figsize=(10, 6))
nodes = range(2, clientCount + 1)
plt.plot(nodes, lostClientsRatios, 'bo-', markerfacecolor='red', markersize=8)
plt.xlabel('Node Number')
plt.ylabel('Lost Clients Ratio (%)')
plt.title('Lost Clients Ratio by Node Number')
plt.grid(True)
plt.savefig('Lost_Clients_Ratio.png')  # Lưu biểu đồ dưới dạng file ảnh
plt.show()  # Hiển thị biểu đồ

# Ghi dữ liệu vào file CSV nếu được yêu cầu
if len(sys.argv) > 2:
    with open(f"summarizedData/{sys.argv[2]}.csv", mode="a") as file:
        for ratio in lostClientsRatios:
            file.write(f"{ratio:.2f}\n")