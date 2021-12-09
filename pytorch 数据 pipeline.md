## pytorch 数据 pipeline

### pytorch data pipeline 学习

pytorch中的 data  pipeline 分为 Sampler、Dataset、DataloaderIter、Dataloader 四个抽象层次。

#### Sampler

Sampler负责生成读取的index序列，在LaneGCN中在DistributedSampler里自定义了index序列的方式。

#### Dataset

Dataset 负责根据 index 查找相应数据并执行预处理，其中有两个私有成员（\_\_getitem\_\_和\_\_len\_\_）变量必须被重载，否则会触发错误提示：

#def \_\_init\_\_()，此函数得到数据的路径

#def \_\_getitem\_\_(self,index)，通过\_\_getitem\_\_函数可以实现通过索引返回对应数据的功能。

#def \_\_len\_\_(self)

#### DataloaderIter

DataloaderIter负责协调多进程执行Dataset

#### Dataloader

Dataloader是最顶层的抽象，Dataloader 作为迭代器，最基本的使用就是传入一个 Dataset 对象，根据sampler确定的取样本顺序，通过 Dataset 类里面的 \_\_getitem\_\_ 函数获取单个的数据，根据参数 batch_size 的值组合成一个 batch 的数据；然后传递给 collate_fn 所指定的函数对这个 batch 做一些操作，比如 padding 之类的。

```python
DataLoader(dataset, batch_size=1, shuffle=False, sampler=None,
           batch_sampler=None, num_workers=0, collate_fn=None,
           pin_memory=False, drop_last=False, timeout=0,
           worker_init_fn=None)
```

**dataset：** torch.utils.data.Dataset 类的实例。也就是说为了使用 DataLoader 类，需要先定义一个 torch.utils.data.Dataset 类的实例。

**shuffle：**如果设置为 True 表示训练样本数据会被随机打乱，默认值为 False 。

**sampler：**自定义从数据集中取样本的策略，如果指定这个参数，那么 shuffle 必须为 False 。从源码中可以看到，如果指定了该参数，同时 shuffle 设定为 True，DataLoader 的 __init__ 函数就会抛出一个异常 。

**collate_fn：**用来打包 batch

**num_worker**：多线程数

具体实现上，Dataloader 其实就是 DataLoaderIter的一个框架，通过 \_\_iter\_\_()函数，把自己装进 DataloaderIter 里。

1. 在\_\_iter\_\_() 中，产生一个DataloaderIter。
2. 通过反复调用 DataloaderIter 的 \_\_next\_\_() 来得到 batch，具体操作就是，多次调用 Dataset 中的 \_\_getitem\_\_ 方法（如果 num_worker>0 就多线程调用），然后用 `collate_fn`来把它们打包成 batch，中间还会涉及到 shuffle，以及 sample 的方法等。
3. 数据读完后，\_\_next\_\_() 抛出一个`StopIteration`异常, `for`循环结束, `dataloader` 失效。
